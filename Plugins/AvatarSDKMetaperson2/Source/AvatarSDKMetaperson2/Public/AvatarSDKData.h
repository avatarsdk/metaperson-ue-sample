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