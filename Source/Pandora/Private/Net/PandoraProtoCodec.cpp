// Copyright Pandora. All Rights Reserved.

#include "Net/PandoraProtoCodec.h"

// ============================================================================
// FPandoraProtoWriter
// ============================================================================

void FPandoraProtoWriter::WriteVarintRaw(uint64 Value)
{
	// base-128 varint：每字节?7 位存数据，最高位?后续还有字节"标志?
	while (Value >= 0x80)
	{
		Buffer.Add(static_cast<uint8>(Value) | 0x80);
		Value >>= 7;
	}
	Buffer.Add(static_cast<uint8>(Value));
}

void FPandoraProtoWriter::WriteTag(int32 FieldNumber, uint8 WireType)
{
	const uint64 Tag = (static_cast<uint64>(FieldNumber) << 3) | (WireType & 0x7);
	WriteVarintRaw(Tag);
}

void FPandoraProtoWriter::WriteUInt64(int32 FieldNumber, uint64 Value)
{
	WriteTag(FieldNumber, PandoraProto::WireVarint);
	WriteVarintRaw(Value);
}

void FPandoraProtoWriter::WriteInt64(int32 FieldNumber, int64 Value)
{
	// proto3 int64：负数按 10 字节 two's-complement varint 编码，直接位?uint64 即可?
	WriteTag(FieldNumber, PandoraProto::WireVarint);
	WriteVarintRaw(static_cast<uint64>(Value));
}

void FPandoraProtoWriter::WriteInt32(int32 FieldNumber, int32 Value)
{
	// proto3 int32：负数同样符号扩展到 64 位再 varint（与官方实现一致）?
	WriteTag(FieldNumber, PandoraProto::WireVarint);
	WriteVarintRaw(static_cast<uint64>(static_cast<int64>(Value)));
}

void FPandoraProtoWriter::WriteString(int32 FieldNumber, const FString& Value)
{
	FTCHARToUTF8 Utf8(*Value);
	WriteTag(FieldNumber, PandoraProto::WireLen);
	WriteVarintRaw(static_cast<uint64>(Utf8.Length()));
	AppendRaw(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
}

void FPandoraProtoWriter::WriteBytes(int32 FieldNumber, const TArray<uint8>& Value)
{
	WriteTag(FieldNumber, PandoraProto::WireLen);
	WriteVarintRaw(static_cast<uint64>(Value.Num()));
	AppendRaw(Value.GetData(), Value.Num());
}

void FPandoraProtoWriter::AppendRaw(const uint8* InData, int32 Num)
{
	if (InData && Num > 0)
	{
		Buffer.Append(InData, Num);
	}
}

// ============================================================================
// FPandoraProtoReader
// ============================================================================

bool FPandoraProtoReader::ReadVarintRaw(uint64& OutValue)
{
	uint64 Result = 0;
	int32 Shift = 0;
	while (Shift < 64)
	{
		if (Pos >= Len)
		{
			return false; // 越界：数据被截断
		}
		const uint8 Byte = Data[Pos++];
		Result |= static_cast<uint64>(Byte & 0x7F) << Shift;
		if ((Byte & 0x80) == 0)
		{
			OutValue = Result;
			return true;
		}
		Shift += 7;
	}
	return false; // varint 超过 10 字节，视为损?
}

bool FPandoraProtoReader::ReadTag(int32& OutFieldNumber, uint8& OutWireType)
{
	uint64 Tag = 0;
	if (!ReadVarintRaw(Tag))
	{
		return false;
	}
	OutFieldNumber = static_cast<int32>(Tag >> 3);
	OutWireType = static_cast<uint8>(Tag & 0x7);
	return OutFieldNumber > 0;
}

bool FPandoraProtoReader::ReadInt64(int64& OutValue)
{
	uint64 Raw = 0;
	if (!ReadVarintRaw(Raw))
	{
		return false;
	}
	OutValue = static_cast<int64>(Raw);
	return true;
}

bool FPandoraProtoReader::ReadInt32(int32& OutValue)
{
	uint64 Raw = 0;
	if (!ReadVarintRaw(Raw))
	{
		return false;
	}
	OutValue = static_cast<int32>(static_cast<int64>(Raw));
	return true;
}

bool FPandoraProtoReader::ReadBool(bool& bOutValue)
{
	uint64 Raw = 0;
	if (!ReadVarintRaw(Raw))
	{
		return false;
	}
	bOutValue = (Raw != 0);
	return true;
}

bool FPandoraProtoReader::ReadString(FString& OutValue)
{
	uint64 Length = 0;
	if (!ReadVarintRaw(Length))
	{
		return false;
	}
	if (static_cast<int64>(Length) > static_cast<int64>(Remaining()))
	{
		return false;
	}
	const int32 Num = static_cast<int32>(Length);
	FUTF8ToTCHAR Conv(reinterpret_cast<const ANSICHAR*>(Data + Pos), Num);
	OutValue = FString(Conv.Length(), Conv.Get());
	Pos += Num;
	return true;
}

bool FPandoraProtoReader::ReadBytes(TArray<uint8>& OutValue)
{
	uint64 Length = 0;
	if (!ReadVarintRaw(Length))
	{
		return false;
	}
	if (static_cast<int64>(Length) > static_cast<int64>(Remaining()))
	{
		return false;
	}
	const int32 Num = static_cast<int32>(Length);
	OutValue.Reset(Num);
	OutValue.Append(Data + Pos, Num);
	Pos += Num;
	return true;
}

bool FPandoraProtoReader::SkipField(uint8 WireType)
{
	switch (WireType)
	{
	case PandoraProto::WireVarint:
	{
		uint64 Dummy = 0;
		return ReadVarintRaw(Dummy);
	}
	case PandoraProto::WireI64:
	{
		if (Remaining() < 8) { return false; }
		Pos += 8;
		return true;
	}
	case PandoraProto::WireLen:
	{
		uint64 Length = 0;
		if (!ReadVarintRaw(Length)) { return false; }
		if (static_cast<int64>(Length) > static_cast<int64>(Remaining())) { return false; }
		Pos += static_cast<int32>(Length);
		return true;
	}
	case PandoraProto::WireI32:
	{
		if (Remaining() < 4) { return false; }
		Pos += 4;
		return true;
	}
	default:
		// group?/4）不支持，遇到即视为损坏
		return false;
	}
}
