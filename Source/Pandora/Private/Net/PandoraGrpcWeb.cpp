// Copyright Pandora. All Rights Reserved.

#include "Net/PandoraGrpcWeb.h"

TArray<uint8> FPandoraGrpcWeb::EncodeMessageFrame(const TArray<uint8>& Message)
{
	const int32 Len = Message.Num();
	TArray<uint8> Frame;
	Frame.Reserve(5 + Len);

	Frame.Add(FlagData);
	// 4 字节 big-endian 长度前缀
	Frame.Add(static_cast<uint8>((Len >> 24) & 0xFF));
	Frame.Add(static_cast<uint8>((Len >> 16) & 0xFF));
	Frame.Add(static_cast<uint8>((Len >> 8) & 0xFF));
	Frame.Add(static_cast<uint8>(Len & 0xFF));
	Frame.Append(Message);

	return Frame;
}

void FPandoraGrpcWebStreamParser::Feed(const uint8* InData, int64 InLength)
{
	if (InData && InLength > 0)
	{
		Buffer.Append(InData, static_cast<int32>(InLength));
	}
}

bool FPandoraGrpcWebStreamParser::NextFrame(uint8& OutFlags, TArray<uint8>& OutPayload)
{
	const int32 Available = Buffer.Num() - Cursor;
	if (Available < 5)
	{
		return false; // 连帧头都不够
	}

	const uint8* Base = Buffer.GetData() + Cursor;
	const uint8 Flags = Base[0];
	const uint32 Length =
		(static_cast<uint32>(Base[1]) << 24) |
		(static_cast<uint32>(Base[2]) << 16) |
		(static_cast<uint32>(Base[3]) << 8) |
		(static_cast<uint32>(Base[4]));

	if (static_cast<int64>(Available - 5) < static_cast<int64>(Length))
	{
		return false; // 帧体还没收全
	}

	OutFlags = Flags;
	OutPayload.Reset(static_cast<int32>(Length));
	OutPayload.Append(Base + 5, static_cast<int32>(Length));

	Cursor += 5 + static_cast<int32>(Length);
	CompactIfNeeded();
	return true;
}

bool FPandoraGrpcWebStreamParser::ParseTrailer(const TArray<uint8>& TrailerPayload, int32& OutStatus, FString& OutMessage)
{
	OutStatus = -1;
	OutMessage.Reset();

	if (TrailerPayload.Num() == 0)
	{
		return false;
	}

	// trailer ?ascii 文本，形?"grpc-status:0\r\ngrpc-message:OK\r\n"（header 名大小写不敏感）?
	FUTF8ToTCHAR Conv(reinterpret_cast<const ANSICHAR*>(TrailerPayload.GetData()), TrailerPayload.Num());
	const FString Text(Conv.Length(), Conv.Get());

	TArray<FString> Lines;
	Text.ParseIntoArrayLines(Lines, /*bCullEmpty*/ true);
	for (const FString& Line : Lines)
	{
		int32 ColonIdx = INDEX_NONE;
		if (!Line.FindChar(TEXT(':'), ColonIdx))
		{
			continue;
		}
		const FString Key = Line.Left(ColonIdx).TrimStartAndEnd().ToLower();
		const FString Value = Line.Mid(ColonIdx + 1).TrimStartAndEnd();

		if (Key == TEXT("grpc-status"))
		{
			OutStatus = FCString::Atoi(*Value);
		}
		else if (Key == TEXT("grpc-message"))
		{
			OutMessage = Value;
		}
	}

	return OutStatus != -1;
}

void FPandoraGrpcWebStreamParser::Reset()
{
	Buffer.Reset();
	Cursor = 0;
}

void FPandoraGrpcWebStreamParser::CompactIfNeeded()
{
	// Cursor 走过半且已消?>4KB 时，把未消费尾部搬到头部，回收内存?
	if (Cursor > 4096 && Cursor * 2 >= Buffer.Num())
	{
		const int32 Tail = Buffer.Num() - Cursor;
		if (Tail > 0)
		{
			FMemory::Memmove(Buffer.GetData(), Buffer.GetData() + Cursor, Tail);
		}
		Buffer.SetNum(Tail, /*bAllowShrinking*/ false);
		Cursor = 0;
	}
}
