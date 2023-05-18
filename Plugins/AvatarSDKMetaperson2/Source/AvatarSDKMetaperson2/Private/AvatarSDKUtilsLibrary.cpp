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

    return Result;
}
