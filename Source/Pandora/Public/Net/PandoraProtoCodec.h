// Copyright Pandora. All Rights Reserved.
//
// 极简 protobuf wire-format 编解码器?客户端零额外依赖"路线）?
//
// 背景：本项目客户?*不引?* grpc-cpp / protobuf C++ 运行库（避免 80MB+ 依赖和插件维护）?
// 改用 UE 自带 FHttpModule ?gRPC-Web，手写极简 protobuf wire 编解码?
// 只实现后?login / push 协议当前用到的字段类型，够用即可，不追求完整 protobuf 实现?
//   - wire type 0（varint）：uint64 / int64 / int32 / enum
//   - wire type 2（length-delimited）：string（UTF-8? bytes / 嵌套 message
// 未用到的 wire type 1（I64? 5（I32? packed repeated 暂不实现，仅?Skip 时跳过?
//
// 字段读取??tag 扫描 + 未知字段 Skip"风格（protobuf 前向兼容惯例），
// 调用方不要假设字段顺序，?field number 分发?

#pragma once

#include "CoreMinimal.h"

namespace PandoraProto
{
	enum EWireType : uint8
	{
		WireVarint = 0,  // int32/int64/uint32/uint64/sint/bool/enum
		WireI64    = 1,  // fixed64/sfixed64/double（本项目未用，仅 Skip?
		WireLen    = 2,  // string/bytes/嵌套 message/packed repeated
		WireStart  = 3,  // group start（deprecated，不支持?
		WireEnd    = 4,  // group end（deprecated，不支持?
		WireI32    = 5,  // fixed32/sfixed32/float（本项目未用，仅 Skip?
	};
}

/**
 * protobuf 写入器：把字段按 wire-format 追加到内部字节缓冲?
 * 用法：依?WriteXxx(field_number, value)，最?GetBytes() 取序列化结果?
 * 注意：只?非默认?字段更省字节，但 proto3 允许显式写默认值，本类按调用方意图写?
 */
class PANDORA_API FPandoraProtoWriter
{
public:
	void WriteVarintRaw(uint64 Value);
	void WriteTag(int32 FieldNumber, uint8 WireType);

	void WriteUInt64(int32 FieldNumber, uint64 Value);
	void WriteInt64(int32 FieldNumber, int64 Value);
	void WriteInt32(int32 FieldNumber, int32 Value);
	void WriteEnum(int32 FieldNumber, int32 Value) { WriteInt32(FieldNumber, Value); }
	void WriteBool(int32 FieldNumber, bool bValue) { WriteUInt64(FieldNumber, bValue ? 1u : 0u); }

	// string：UTF-8 编码后作 length-delimited 写入?
	void WriteString(int32 FieldNumber, const FString& Value);
	// bytes：原?length-delimited 写入?
	void WriteBytes(int32 FieldNumber, const TArray<uint8>& Value);

	const TArray<uint8>& GetBytes() const { return Buffer; }
	TArray<uint8> MoveBytes() { return MoveTemp(Buffer); }

private:
	void AppendRaw(const uint8* Data, int32 Num);
	TArray<uint8> Buffer;
};

/**
 * protobuf 读取器：在外部持有的只读字节缓冲上游标式解析?
 * 典型循环?
 *   while (!R.AtEnd()) {
 *     int32 Field; uint8 Wire;
 *     if (!R.ReadTag(Field, Wire)) break;
 *     switch (Field) { case 2: R.ReadUInt64(PlayerId); break; default: R.SkipField(Wire); }
 *   }
 * 任一 ReadXxx 返回 false 表示数据越界/损坏，调用方应中止解析并按损坏处理?
 */
class PANDORA_API FPandoraProtoReader
{
public:
	FPandoraProtoReader(const uint8* InData, int32 InLen)
		: Data(InData), Len(InLen), Pos(0) {}

	explicit FPandoraProtoReader(const TArray<uint8>& In)
		: Data(In.GetData()), Len(In.Num()), Pos(0) {}

	bool AtEnd() const { return Pos >= Len; }
	int32 Remaining() const { return Len - Pos; }

	bool ReadVarintRaw(uint64& OutValue);
	bool ReadTag(int32& OutFieldNumber, uint8& OutWireType);

	bool ReadUInt64(uint64& OutValue) { return ReadVarintRaw(OutValue); }
	bool ReadInt64(int64& OutValue);
	bool ReadInt32(int32& OutValue);
	bool ReadEnum(int32& OutValue) { return ReadInt32(OutValue); }
	bool ReadBool(bool& bOutValue);

	bool ReadString(FString& OutValue);
	bool ReadBytes(TArray<uint8>& OutValue);

	// 跳过一个当前不关心 / 未知的字段（?wire type 决定跳过多少字节）?
	bool SkipField(uint8 WireType);

private:
	const uint8* Data;
	int32 Len;
	int32 Pos;
};
