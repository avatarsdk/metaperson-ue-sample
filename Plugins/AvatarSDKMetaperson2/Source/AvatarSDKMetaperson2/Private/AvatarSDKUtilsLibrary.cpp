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
#include "AndroidUtils.h"
#include "Misc/Paths.h"

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
    int32 CharIndex;
    auto SearchPos = Url.FindLastChar('/', CharIndex);
    check(SearchPos);
    auto FileName = Url.RightChop(CharIndex + 1); // archive file name
    auto LeftPart = Url.LeftChop(Url.Len() - CharIndex); // left part without archive file name
    SearchPos = LeftPart.FindLastChar('/', CharIndex); // index of '/' in the left part
    check(SearchPos);
    auto Code = LeftPart.RightChop(CharIndex + 1); // avatar code

    FAvatarData Result;
    Result.Url = Url;
    Result.Code = Code;
    Result.DirectoryPath = FPaths::Combine(GetCommonDataFolder(), TEXT("avatars"), Code);
    Result.ArchivePath = FPaths::Combine(Result.DirectoryPath, FileName);
    Result.ModelPath = FPaths::Combine(Result.DirectoryPath, TEXT("model.glb"));
    return Result;
}
