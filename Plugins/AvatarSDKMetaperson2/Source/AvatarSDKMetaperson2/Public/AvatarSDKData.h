#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AvatarSDKData.generated.h"

USTRUCT(BlueprintType)
struct AVATARSDKMETAPERSON2_API FAvatarData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite, Category = "Default")
    FString Code;
    UPROPERTY(BlueprintReadWrite, Category = "Default")
    FString DirectoryPath;
    UPROPERTY(BlueprintReadWrite, Category = "Default")
    FString ModelPath;
    UPROPERTY(BlueprintReadWrite, Category = "Default")
    FString ArchivePath;
    UPROPERTY(BlueprintReadWrite, Category = "Default")
    FString Url;
};