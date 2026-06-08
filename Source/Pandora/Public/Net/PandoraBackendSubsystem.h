// Copyright Pandora. All Rights Reserved.
//
// UPandoraBackendSubsystem —?UE 客户端连后端的高层入口（GameInstanceSubsystem，全局单例）?
//
// 铁律对齐（Pandora HANDOFF §1.1 客户端连?2 条铁律）?
//   ?UE NetDriver ?Hub/Battle DS：游戏内 GAS / Replication?*不在本类**，走引擎原生网络?
//   ?FHttpModule ?Envoy(:8443 HTTPS)：业?unary + 推?server stream，gRPC-Web over HTTP/2 TLS?*本类**?
//
// 本类只负责铁?②：
//   - Login()      ：unary，POST /pandora.login.v1.LoginService/Login，拿 session_token + hub_ds_addr/hub_ticket
//   - Subscribe()  ：server stream，POST /pandora.push.v1.PushService/Subscribe，长连接?PushFrame
//   - CloseStream()：主动断开推送长连接
//
// 协议细节?docs（后端仓库）?
//   - Content-Type: application/grpc-web+proto
//   - 需鉴权?path（Subscribe / Logout / IssueDSTicket）带 Authorization: Bearer <session_token>
//   - push.Subscribe 永不超时（Envoy route timeout=0），客户端靠 CloseStream / 断线重连管理生命周期

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "PandoraBackendSubsystem.generated.h"

class FPandoraGrpcWebStreamParser;

/** Login 结果（透明区分 传输?/ gRPC ?/ 业务?三级状态）?*/
USTRUCT(BlueprintType)
struct FPandoraLoginResult
{
	GENERATED_BODY()

	// 传输层：HTTP/gRPC-Web 请求是否成功送达并拿到响应?
	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	bool bTransportOk = false;

	// gRPC 层：trailer 里的 grpc-status?=OK），传输失败时为 -1?
	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	int32 GrpcStatus = -1;

	// 业务层：LoginResponse.code（pandora.common.v1.ErrCode?=成功）?
	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	int32 Code = 0;

	// 注意：后?player_id ?uint64（snowflake?3 位，?< int64 max），UE/BP 侧用 int64 承载安全?
	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	int64 PlayerId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	FString SessionToken;

	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	FString HubDsAddr;

	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	FString HubTicket;

	// 人类可读的失败原因（传输失败 / grpc-status != 0 / 解析失败时填充）?
	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	FString Error;
};

/** 一帧推送（对应后端 pandora.push.v1.PushFrame）?*/
USTRUCT(BlueprintType)
struct FPandoraPushFrame
{
	GENERATED_BODY()

	// kafka topic，客户端据此分发到不?UI handler（如 pandora.team.update）?
	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	FString Topic;

	// 业务 Event message ?protobuf 字节，按 topic 选对?message 反序列化?
	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	TArray<uint8> Payload;

	// 事件生产时间（毫秒），客户端按它去重（kafka at-least-once? 断线重连补推游标?
	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	int64 TsMs = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pandora|Backend")
	FString TraceId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPandoraOnLoginComplete, const FPandoraLoginResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPandoraOnPushFrame, const FPandoraPushFrame&, Frame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPandoraOnStreamClosed, const FString&, Reason);

/**
 * 后端连接子系统。通过 GetGameInstance()->GetSubsystem<UPandoraBackendSubsystem>() 获取?
 */
UCLASS(Config = Game)
class PANDORA_API UPandoraBackendSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// === 网关配置（默认指向本?Envoy；上线改 Config/DefaultGame.ini 或运行时 SetGateway?==
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Pandora|Backend")
	FString GatewayHost = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Pandora|Backend")
	int32 GatewayPort = 8443;

	// dev 自签证书：UE HTTP 默认会校?TLS 证书链。本?Envoy 用自签证书时需把证?
	// 加入系统受信任根，或在打包配置里放开校验。详?cpp 顶部说明（这里仅作展示标记）?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Pandora|Backend")
	bool bDevInsecureTls = true;

	// === 事件 ===
	UPROPERTY(BlueprintAssignable, Category = "Pandora|Backend")
	FPandoraOnLoginComplete OnLoginComplete;

	UPROPERTY(BlueprintAssignable, Category = "Pandora|Backend")
	FPandoraOnPushFrame OnPushFrame;

	UPROPERTY(BlueprintAssignable, Category = "Pandora|Backend")
	FPandoraOnStreamClosed OnStreamClosed;

	// === API ===

	// 立即完成?unary。结果通过 OnLoginComplete 广播（游戏线程）?
	// PasswordHash：调用方先对明文密码?sha256（后?LoginRequest.password_hash 语义）?
	UFUNCTION(BlueprintCallable, Category = "Pandora|Backend")
	void Login(const FString& Account, const FString& PasswordHash, const FString& DeviceId,
		const FString& ClientVersion, const FString& Region, const FString& Locale);

	// 已受?+ 长连。须?Login 成功（用已存?SessionToken）?
	// LastSeenMs：断线重连补推游标，首连?0。每帧经 OnPushFrame 广播?
	UFUNCTION(BlueprintCallable, Category = "Pandora|Backend")
	void Subscribe(int64 LastSeenMs = 0);

	// 主动关闭推送长连接?
	UFUNCTION(BlueprintCallable, Category = "Pandora|Backend")
	void CloseStream();

	UFUNCTION(BlueprintCallable, Category = "Pandora|Backend")
	void SetGateway(const FString& Host, int32 Port);

	UFUNCTION(BlueprintPure, Category = "Pandora|Backend")
	int64 GetPlayerId() const { return PlayerId; }

	UFUNCTION(BlueprintPure, Category = "Pandora|Backend")
	FString GetSessionToken() const { return SessionToken; }

	UFUNCTION(BlueprintPure, Category = "Pandora|Backend")
	bool IsStreamActive() const { return StreamRequest.IsValid(); }

	// UGameInstanceSubsystem
	virtual void Deinitialize() override;

private:
	// ?gRPC 全路?URL：https://<host>:<port>/<FullMethod>
	FString BuildUrl(const FString& FullMethod) const;

	// 构造一个带 gRPC-Web ?+ 帧化请求体的 HTTP 请求（bWithAuth 时附 Authorization）?
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> MakeGrpcWebRequest(
		const FString& FullMethod, const TArray<uint8>& MessageBytes, bool bWithAuth) const;

	// unary login 完成回调?
	void OnLoginHttpComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

	// server stream 增量字节回调（可能在非游戏线程触发）。返?false 可中止接收?
	void OnStreamBytes(void* DataPtr, int64& DataLen);

	// server stream 整体结束（正常断?/ 网络错误）回调?
	void OnStreamHttpComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

	void BroadcastStreamClosed(const FString& Reason);

	// 解析一?PushFrame protobuf 字节到结构体?
	static bool ParsePushFrame(const TArray<uint8>& Payload, FPandoraPushFrame& OutFrame);
	// 解析 LoginResponse protobuf 字节到结果?
	static bool ParseLoginResponse(const TArray<uint8>& Payload, FPandoraLoginResult& OutResult);

	UPROPERTY()
	FString SessionToken;

	int64 PlayerId = 0;

	// 推送流状态（StreamParser 在接收线程写，广?marshal 回游戏线程）?
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> StreamRequest;
	TSharedPtr<FPandoraGrpcWebStreamParser> StreamParser;
	int32 StreamTrailerStatus = -1;
	FString StreamTrailerMessage;
};
