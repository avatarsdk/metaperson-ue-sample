/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, May 2023
 */

#include "AvatarSDKUtilsLibrary.h"

FString UAvatarSDKUtilsLibrary::GetCommonDataFolder()
{
#if PLATFORM_ANDROID
    UAndroidUtils* androidUtils = NewObject<UAndroidUtils>();
    return androidUtils->GetPersistentDataPath();
#elif PLATFORM_IOS
    return[PhotoViewController getPersistentDataPath];
#else
    return FPaths::Combine(FPlatformProcess::UserSettingsDir(), FString("Avatar SDK Metaperson 2"));
#endif
}

FAvatarData UAvatarSDKUtilsLibrary::GetAvatarData(const FString& Url)
{
    const FString AvatatsToken = TEXT("/avatars/");    
    auto SearchPos = Url.Find(AvatatsToken);
    check(SearchPos > 0);
    auto RightPart = Url.RightChop(SearchPos + AvatatsToken.Len());
    FString Code, FileName;
    RightPart.Split(TEXT("/"), &Code, &FileName);
    
    FAvatarData Result;
    Result.Url = Url;
    Result.Code = Code;
    Result.DirectoryPath = FPaths::Combine(GetCommonDataFolder(), TEXT("avatars"), Code);
    Result.ArchivePath = FPaths::Combine(Result.DirectoryPath, FileName);
    Result.ModelPath = FPaths::Combine(Result.DirectoryPath, TEXT("model.glb"));
    return Result;
}
