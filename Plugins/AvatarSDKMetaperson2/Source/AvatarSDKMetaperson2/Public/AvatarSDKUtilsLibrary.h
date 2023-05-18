#pragma once
#include "CoreMinimal.h"
#include "AvatarSDKData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AvatarSDKUtilsLibrary.generated.h"

UCLASS()
class AVATARSDKMETAPERSON2_API UAvatarSDKUtilsLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AvatarSDK|Metaperson2")
	static FString GetCommonDataFolder();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AvatarSDK|Metaperson2")
	static FAvatarData GetAvatarData(const FString& Url);

};