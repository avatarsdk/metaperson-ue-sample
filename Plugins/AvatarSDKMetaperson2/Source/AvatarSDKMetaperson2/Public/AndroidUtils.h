/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, February 2023
 */


#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Async/Future.h"


#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#endif
#include "AndroidUtils.generated.h"

DECLARE_DELEGATE_OneParam(FAndroidFileDelegate, FString);

UCLASS()
class AVATARSDKMETAPERSON2_API UAndroidUtils : public UObject
{
	GENERATED_BODY()

public:
	FAndroidFileDelegate FileDelegate;
	UAndroidUtils();
	UFUNCTION()
		void OpenGalleryAsync();
	void TakeCameraImageAsync();
	FString GetPersistentDataPath();

#if PLATFORM_ANDROID
	void OnActivityResult(
		JNIEnv* jenv,
		jobject thiz,
		jobject activity,
		jint requestCode,
		jint resultCode,
		jobject data
	);
#endif
private:
	FDelegateHandle DelegateHandle;
#if PLATFORM_ANDROID
	jmethodID OpenGalleryJMethod;
	jmethodID GetPersistentDataPathJMethod;
	jmethodID GetImagePathJMethod;
	jmethodID TakeCameraImageJMethod;
	bool CheckPermissions();
#endif
};