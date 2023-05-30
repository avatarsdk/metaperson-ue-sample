/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, May 2023
 */


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