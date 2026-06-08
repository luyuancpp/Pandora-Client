// Copyright Pandora. All Rights Reserved.
//
// gRPC-Web 帧（frame）编解码 + 流式解析器?
//
// gRPC-Web wire 格式（Envoy envoy.filters.http.grpc_web 与之互转标准 gRPC）：
//   每一?= [1 字节 flags][4 字节 big-endian 长度][payload...]
//     - flags 最高位 (0x80) = 1：trailer 帧（HTTP/1 风格?ascii 文本，含 grpc-status / grpc-message?
//     - flags 最高位     = 0：data 帧（payload 是一?protobuf 序列化消息）
//   压缩位（flags & 0x01）本项目不启用（请求/响应都不压缩）?
//
// unary（login）：响应?= 1 ?data ?+ 1 ?trailer 帧，一次性到达?
// server stream（push.Subscribe）：响应?= 持续到达的多?data 帧，最后（断流时）一?trailer 帧?
//   ?必须?FPandoraGrpcWebStreamParser 边收边解析（HTTP 回包是分块流式到达的）?

#pragma once

#include "CoreMinimal.h"

class PANDORA_API FPandoraGrpcWeb
{
public:
	static constexpr uint8 FlagData    = 0x00;
	static constexpr uint8 FlagTrailer = 0x80;

	// 把一?protobuf 消息包成一?gRPC-Web data 帧（请求体用）?
	static TArray<uint8> EncodeMessageFrame(const TArray<uint8>& Message);
};

/**
 * 流式帧解析器：把陆续到达?HTTP 响应字节喂进来，按完整帧吐出?
 *
 * 用法?
 *   Parser.Feed(Ptr, Len);
 *   uint8 Flags; TArray<uint8> Payload;
 *   while (Parser.NextFrame(Flags, Payload)) {
 *     if (Flags & FlagTrailer) { ParseTrailer(Payload, Status, Msg); }
 *     else                     { // data ??反序列化业务 message }
 *   }
 *
 * 线程注意：UE HTTP 的流式接收回调可能在非游戏线程触发，本类自身不加锁，
 * 调用方（Subsystem）负责把解析结果 marshal 回游戏线程再广播?UI/BP?
 */
class PANDORA_API FPandoraGrpcWebStreamParser
{
public:
	// 追加一段原始响应字节?
	void Feed(const uint8* InData, int64 InLength);

	// 取出下一个完整帧；缓冲里没有完整帧时返回 false（保留半帧等后续 Feed）?
	bool NextFrame(uint8& OutFlags, TArray<uint8>& OutPayload);

	// 解析 trailer 帧文本（"grpc-status:0\r\ngrpc-message:...\r\n"），抽出状态码与消息?
	// 找不?grpc-status ?OutStatus 保持 -1?
	static bool ParseTrailer(const TArray<uint8>& TrailerPayload, int32& OutStatus, FString& OutMessage);

	void Reset();

private:
	void CompactIfNeeded();

	TArray<uint8> Buffer; // 累积未消费字?
	int32 Cursor = 0;     // 已消费偏移（延迟 compact，避免频繁内存搬移）
};
