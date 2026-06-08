// Copyright Pandora. All Rights Reserved.
//
// === 关于本地自签 TLS 证书（dev?==
// 本地 Envoy(:8443) 用自签证书时，UE HTTP（libcurl）默认会因证书链不受信而拒连?
// dev 期处理方式（任选其一，本类代码不强制）：
//   1) ?Envoy 自签 CA 导入 Windows 受信任根证书颁发机构（推荐，零代码）?
//   2) 用受信任 CA（如 mkcert 生成）签?Envoy 证书?
//   3) 打包配置里放开校验（仅限内部测试包，切勿上线）?
// bDevInsecureTls 仅作为意图标记，真正放开校验需?HTTP/平台层处理，不在本类硬编码绕过?
//
// === 关于 HTTP 版本 ===
// gRPC-Web 设计上同时兼?HTTP/1.1（chunked，可承载 server stream）与 HTTP/2?
// libcurl 通过 TLS ALPN 自动?Envoy 协商 h2/http1.1（Envoy listener alpn = ["h2","http/1.1"]），
// 因此本类不需要手动指?HTTP 版本?

#include "Net/PandoraBackendSubsystem.h"

#include "Net/PandoraProtoCodec.h"
#include "Net/PandoraGrpcWeb.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Async/Async.h"

DEFINE_LOG_CATEGORY_STATIC(LogPandoraBackend, Log, All);

namespace
{
	const TCHAR* MethodLogin     = TEXT("/pandora.login.v1.LoginService/Login");
	const TCHAR* MethodSubscribe = TEXT("/pandora.push.v1.PushService/Subscribe");
}

void UPandoraBackendSubsystem::Deinitialize()
{
	CloseStream();
	Super::Deinitialize();
}

void UPandoraBackendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!bAutoLoginForDev || bDevAutoLoginStarted || IsRunningDedicatedServer())
	{
		return;
	}

	bDevAutoLoginStarted = true;
	UE_LOG(LogPandoraBackend, Warning,
		TEXT("Dev auto login started: https://%s:%d account=%s"),
		*GatewayHost, GatewayPort, *DevLoginAccount);

	Login(
		DevLoginAccount,
		DevLoginPasswordHash,
		DevLoginDeviceId,
		DevLoginClientVersion,
		DevLoginRegion,
		DevLoginLocale);
}

void UPandoraBackendSubsystem::SetGateway(const FString& Host, int32 Port)
{
	GatewayHost = Host;
	GatewayPort = Port;
}

FString UPandoraBackendSubsystem::BuildUrl(const FString& FullMethod) const
{
	return FString::Printf(TEXT("https://%s:%d%s"), *GatewayHost, GatewayPort, *FullMethod);
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UPandoraBackendSubsystem::MakeGrpcWebRequest(
	const FString& FullMethod, const TArray<uint8>& MessageBytes, bool bWithAuth) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BuildUrl(FullMethod));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/grpc-web+proto"));
	Request->SetHeader(TEXT("X-Grpc-Web"), TEXT("1"));
	Request->SetHeader(TEXT("X-User-Agent"), TEXT("Pandora-ue/1.0"));

	if (bWithAuth && !SessionToken.IsEmpty())
	{
		// Envoy jwt_authn 默认?Authorization: Bearer <jwt> ?token 校验?
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *SessionToken));
	}

	// 请求?= 单个 gRPC-Web data 帧（包住一?protobuf 消息）?
	Request->SetContent(FPandoraGrpcWeb::EncodeMessageFrame(MessageBytes));
	return Request;
}

// ============================================================================
// Login（unary?
// ============================================================================

void UPandoraBackendSubsystem::Login(const FString& Account, const FString& PasswordHash, const FString& DeviceId,
	const FString& ClientVersion, const FString& Region, const FString& Locale)
{
	// 序列?LoginRequest（field 号对?proto/pandora/login/v1/login.proto）?
	FPandoraProtoWriter W;
	if (!Account.IsEmpty())       { W.WriteString(1, Account); }
	if (!PasswordHash.IsEmpty())  { W.WriteString(2, PasswordHash); }
	if (!DeviceId.IsEmpty())      { W.WriteString(3, DeviceId); }
	if (!ClientVersion.IsEmpty()) { W.WriteString(4, ClientVersion); }
	if (!Region.IsEmpty())        { W.WriteString(10, Region); }
	if (!Locale.IsEmpty())        { W.WriteString(11, Locale); }

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		MakeGrpcWebRequest(MethodLogin, W.GetBytes(), /*bWithAuth*/ false);

	Request->OnProcessRequestComplete().BindUObject(this, &UPandoraBackendSubsystem::OnLoginHttpComplete);
	Request->ProcessRequest();
}

void UPandoraBackendSubsystem::OnLoginHttpComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	FPandoraLoginResult Result;

	if (!bSucceeded || !Response.IsValid())
	{
		Result.bTransportOk = false;
		Result.Error = TEXT("HTTP request failed");
		UE_LOG(LogPandoraBackend, Warning, TEXT("Login transport failed"));
		OnLoginComplete.Broadcast(Result);
		return;
	}

	Result.bTransportOk = true;
	const int32 HttpCode = Response->GetResponseCode();

	// 解析 gRPC-Web 响应体：data 帧（LoginResponse? trailer 帧（grpc-status）?
	const TArray<uint8>& Body = Response->GetContent();
	FPandoraGrpcWebStreamParser Parser;
	Parser.Feed(Body.GetData(), Body.Num());

	FString TrailerMessage;
	uint8 Flags = 0;
	TArray<uint8> Payload;
	while (Parser.NextFrame(Flags, Payload))
	{
		if (Flags & FPandoraGrpcWeb::FlagTrailer)
		{
			FPandoraGrpcWebStreamParser::ParseTrailer(Payload, Result.GrpcStatus, TrailerMessage);
		}
		else
		{
			ParseLoginResponse(Payload, Result);
		}
	}

	if (Result.GrpcStatus > 0)
	{
		Result.Error = FString::Printf(TEXT("grpc-status=%d %s"), Result.GrpcStatus, *TrailerMessage);
	}
	else if (HttpCode != 200 && Result.GrpcStatus == -1)
	{
		// Envoy 在到达业务服前拒绝（?jwt 失败）时可能不带 grpc-web trailer?
		Result.Error = FString::Printf(TEXT("HTTP %d rejected by gateway without gRPC trailer"), HttpCode);
	}

	// 业务+gRPC 双成功才落地会话态?
	if (Result.GrpcStatus == 0 && Result.Code == 0)
	{
		SessionToken = Result.SessionToken;
		PlayerId = Result.PlayerId;

		if (bAutoLoginForDev)
		{
			UE_LOG(LogPandoraBackend, Warning,
				TEXT("Dev auto login succeeded: player_id=%lld hub=%s; subscribing push stream"),
				PlayerId, *Result.HubDsAddr);
			Subscribe(0);
		}
	}
	else
	{
		UE_LOG(LogPandoraBackend, Warning,
			TEXT("Login failed: transport=%d grpc=%d code=%d error=%s"),
			Result.bTransportOk ? 1 : 0, Result.GrpcStatus, Result.Code, *Result.Error);
	}

	OnLoginComplete.Broadcast(Result);
}

// ============================================================================
// Subscribe（server stream?
// ============================================================================

void UPandoraBackendSubsystem::Subscribe(int64 LastSeenMs)
{
	if (SessionToken.IsEmpty())
	{
		UE_LOG(LogPandoraBackend, Warning, TEXT("Subscribe requires Login first: missing session_token"));
		BroadcastStreamClosed(TEXT("missing_session_token"));
		return;
	}

	if (StreamRequest.IsValid())
	{
		// 顶号：先关旧流再开新流（对齐后?ConnectionManager 顶号语义）?
		CloseStream();
	}

	// 序列?SubscribeRequest（field 号对?proto/pandora/push/v1/push.proto）?
	FPandoraProtoWriter W;
	W.WriteString(1, SessionToken);
	if (LastSeenMs != 0)
	{
		W.WriteInt64(2, LastSeenMs);
	}

	StreamParser = MakeShared<FPandoraGrpcWebStreamParser>();
	StreamTrailerStatus = -1;
	StreamTrailerMessage.Reset();

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		MakeGrpcWebRequest(MethodSubscribe, W.GetBytes(), /*bWithAuth*/ true);

	// 流式接收：每收到一段响应字节回调一次（可能在非游戏线程）?
	// ⚠️ 引擎版本敏感：UE 5.7 ?SetResponseBodyReceiveStreamDelegateV2 / FHttpRequestStreamDelegateV2?
	//   若所用引擎暴露的是非 V2 签名，这里是唯一需要微调的集成点?
	Request->SetResponseBodyReceiveStreamDelegateV2(
		FHttpRequestStreamDelegateV2::CreateUObject(this, &UPandoraBackendSubsystem::OnStreamBytes));

	Request->OnProcessRequestComplete().BindUObject(this, &UPandoraBackendSubsystem::OnStreamHttpComplete);

	StreamRequest = Request;
	Request->ProcessRequest();
}

void UPandoraBackendSubsystem::OnStreamBytes(void* DataPtr, int64& DataLen)
{
	if (!StreamParser.IsValid())
	{
		return; // Stream has already been closed.
	}

	StreamParser->Feed(static_cast<const uint8*>(DataPtr), DataLen);

	TArray<FPandoraPushFrame> Frames;
	uint8 Flags = 0;
	TArray<uint8> Payload;
	while (StreamParser->NextFrame(Flags, Payload))
	{
		if (Flags & FPandoraGrpcWeb::FlagTrailer)
		{
			FPandoraGrpcWebStreamParser::ParseTrailer(Payload, StreamTrailerStatus, StreamTrailerMessage);
		}
		else
		{
			FPandoraPushFrame Frame;
			if (ParsePushFrame(Payload, Frame))
			{
				UE_LOG(LogPandoraBackend, Warning,
					TEXT("PushFrame received: topic=%s payload_bytes=%d ts=%lld trace=%s"),
					*Frame.Topic, Frame.Payload.Num(), Frame.TsMs, *Frame.TraceId);
				Frames.Add(MoveTemp(Frame));
			}
		}
	}

	if (Frames.Num() > 0)
	{
		// 解析在接收线程做，广播到 UI/BP 必须回游戏线程?
		TWeakObjectPtr<UPandoraBackendSubsystem> WeakThis(this);
		AsyncTask(ENamedThreads::GameThread, [WeakThis, MovedFrames = MoveTemp(Frames)]()
		{
			if (UPandoraBackendSubsystem* Self = WeakThis.Get())
			{
				for (const FPandoraPushFrame& F : MovedFrames)
				{
					Self->OnPushFrame.Broadcast(F);
				}
			}
		});
	}
}

void UPandoraBackendSubsystem::OnStreamHttpComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	const FString Reason = bSucceeded
		? FString::Printf(TEXT("stream_ended grpc-status=%d %s"), StreamTrailerStatus, *StreamTrailerMessage)
		: TEXT("stream_transport_error");

	StreamRequest.Reset();
	StreamParser.Reset();
	BroadcastStreamClosed(Reason);
}

void UPandoraBackendSubsystem::CloseStream()
{
	if (StreamRequest.IsValid())
	{
		// CancelRequest 会触?OnStreamHttpComplete(bSucceeded=false) 收尾广播?
		StreamRequest->CancelRequest();
		StreamRequest.Reset();
	}
	StreamParser.Reset();
}

void UPandoraBackendSubsystem::BroadcastStreamClosed(const FString& Reason)
{
	if (IsInGameThread())
	{
		OnStreamClosed.Broadcast(Reason);
	}
	else
	{
		TWeakObjectPtr<UPandoraBackendSubsystem> WeakThis(this);
		AsyncTask(ENamedThreads::GameThread, [WeakThis, Reason]()
		{
			if (UPandoraBackendSubsystem* Self = WeakThis.Get())
			{
				Self->OnStreamClosed.Broadcast(Reason);
			}
		});
	}
}

// ============================================================================
// protobuf message 解析
// ============================================================================

bool UPandoraBackendSubsystem::ParseLoginResponse(const TArray<uint8>& Payload, FPandoraLoginResult& OutResult)
{
	FPandoraProtoReader R(Payload);
	int32 Field = 0;
	uint8 Wire = 0;
	while (!R.AtEnd() && R.ReadTag(Field, Wire))
	{
		switch (Field)
		{
		case 1: { int32 V = 0; if (!R.ReadEnum(V)) return false; OutResult.Code = V; break; }
		case 2: { uint64 V = 0; if (!R.ReadUInt64(V)) return false; OutResult.PlayerId = static_cast<int64>(V); break; }
		case 3: if (!R.ReadString(OutResult.SessionToken)) return false; break;
		case 4: if (!R.ReadString(OutResult.HubDsAddr)) return false; break;
		case 5: if (!R.ReadString(OutResult.HubTicket)) return false; break;
			default: if (!R.SkipField(Wire)) return false; break;
		}
	}
	return true;
}

bool UPandoraBackendSubsystem::ParsePushFrame(const TArray<uint8>& Payload, FPandoraPushFrame& OutFrame)
{
	FPandoraProtoReader R(Payload);
	int32 Field = 0;
	uint8 Wire = 0;
	while (!R.AtEnd() && R.ReadTag(Field, Wire))
	{
		switch (Field)
		{
		case 1: if (!R.ReadString(OutFrame.Topic)) return false; break;
		case 2: if (!R.ReadBytes(OutFrame.Payload)) return false; break;
		case 3: { int64 V = 0; if (!R.ReadInt64(V)) return false; OutFrame.TsMs = V; break; }
		case 4: if (!R.ReadString(OutFrame.TraceId)) return false; break;
			default: if (!R.SkipField(Wire)) return false; break;
		}
	}
	return true;
}
