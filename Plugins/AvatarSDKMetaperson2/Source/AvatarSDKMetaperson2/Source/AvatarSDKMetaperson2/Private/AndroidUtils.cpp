/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, February 2023
 */

#include "AndroidUtils.h"
#include "Async/Async.h"
#include "AvatarSDKMetaperson2.h"
#include "Runtime/Launch/Resources/Version.h"
#if PLATFORM_ANDROID
#include "AndroidPermissionFunctionLibrary.h"
#include "AndroidPermissionCallbackProxy.h"
#endif

UAndroidUtils::UAndroidUtils()
{
	UE_LOG(LogMetaperson2, Warning, TEXT("UAndroidUtils: ctor()"));
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		//OpenGalleryJMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_OpenGallery", "()V", false);
		//GetImagePathJMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GetImagePath", "()Ljava/lang/String;", false);
		GetPersistentDataPathJMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GetPersistentDataPath", "()Ljava/lang/String;", false);
		//TakeCameraImageJMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_TakeCameraPhoto", "()V", false);
	}
#endif

}

#if PLATFORM_ANDROID

void UAndroidUtils::OnActivityResult(
	JNIEnv* jenv,
	jobject thiz,
	jobject activity,
	jint requestCode,
	jint resultCode,
	jobject data
)
{
	int32 code = (int32)requestCode;
	FString imageSelected;
	if (code == 111) // RESULT_LOAD_IMAGE 
	{
		UE_LOG(LogMetaperson2, Log, TEXT("UAndroidUtils: OnActivityResult() Load Image"));

		if ((int32)resultCode == -1) // RESULT_OK
		{
			jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(jenv, FJavaWrapper::GameActivityThis, UAndroidUtils::GetImagePathJMethod);
			const char* JavaChars = jenv->GetStringUTFChars(JavaString, 0);
			imageSelected = FString(UTF8_TO_TCHAR(JavaChars));
			// release string
			jenv->ReleaseStringUTFChars(JavaString, JavaChars);
			jenv->DeleteLocalRef(JavaString);

		}

		AsyncTask(ENamedThreads::GameThread, [this, imageSelected]() {
			FileDelegate.ExecuteIfBound(imageSelected);
			});
	}
	else if (code == 112)
	{
		UE_LOG(LogMetaperson2, Log, TEXT("UAndroidUtils: OnActivityResult() Take Camera Image"));

		if ((int32)resultCode == -1) // RESULT_OK
		{
			jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(jenv, FJavaWrapper::GameActivityThis, UAndroidUtils::GetImagePathJMethod);
			const char* JavaChars = jenv->GetStringUTFChars(JavaString, 0);
			imageSelected = FString(UTF8_TO_TCHAR(JavaChars));
			// release string
			jenv->ReleaseStringUTFChars(JavaString, JavaChars);
			jenv->DeleteLocalRef(JavaString);
		}
		AsyncTask(ENamedThreads::GameThread, [this, imageSelected]() {
			FileDelegate.ExecuteIfBound(imageSelected);
			});
	}
	FJavaWrapper::OnActivityResultDelegate.Remove(DelegateHandle);

}
#endif

void UAndroidUtils::OpenGalleryAsync()
{
	UE_LOG(LogMetaperson2, Warning, TEXT("UAndroidUtils: OpenGallery()"));
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		DelegateHandle = FJavaWrapper::OnActivityResultDelegate.AddUObject(this, &UAndroidUtils::OnActivityResult);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, UAndroidUtils::OpenGalleryJMethod);
	}
#endif
}

FString UAndroidUtils::GetPersistentDataPath()
{
	UE_LOG(LogMetaperson2, Warning, TEXT("UAndroidUtils: GetPersistentDataPath()"));
#if PLATFORM_ANDROID
	FString filePath;
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, UAndroidUtils::GetPersistentDataPathJMethod);
		const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
		filePath = FString(UTF8_TO_TCHAR(JavaChars));

		UE_LOG(LogMetaperson2, Warning, TEXT("UAndroidUtils: GetPersistentDataPath result : %s"), *filePath);
	}
	return filePath;
#else 
	TArray<bool> arr;

	return FString("");
#endif
}

#if PLATFORM_ANDROID
bool UAndroidUtils::CheckPermissions()
{
	TArray <FString> permissionsRequired = { "android.permission.CAMERA", "android.permission.WRITE_EXTERNAL_STORAGE" };
	TArray <FString> permissionsToRequest;
	for (int i = 0; i < permissionsRequired.Num(); i++)
	{
		if (!UAndroidPermissionFunctionLibrary::CheckPermission(permissionsRequired[i]))
		{
			permissionsToRequest.Add(permissionsRequired[i]);
		}
	}

	if (permissionsToRequest.Num())
	{
		if (UAndroidPermissionCallbackProxy* callback = UAndroidPermissionFunctionLibrary::AcquirePermissions(permissionsToRequest))
		{
			TPromise<bool> permissionsPromise;
			auto permissionsFuture = permissionsPromise.GetFuture();
#if ENGINE_MAJOR_VERSION >= 5
			callback->OnPermissionsGrantedDelegate.AddLambda([this, &permissionsPromise, &permissionsToRequest](const TArray<FString>& Permissions, const TArray<bool>& GrantResults)
#else
			callback->OnPermissionsGrantedDelegate.BindLambda([this, &permissionsPromise, &permissionsToRequest](const TArray<FString>& Permissions, const TArray<bool>& GrantResults)
#endif
				{
					bool status = GrantResults.Contains(false) ? false : true;
					permissionsPromise.EmplaceValue(status);
				});
			permissionsFuture.Wait();
			bool permissionsSuccess = permissionsFuture.Get();
			return permissionsSuccess;
		}
	}
	else
	{
		return true;
	}
	return false;
}
#endif

void UAndroidUtils::TakeCameraImageAsync()
{

	UE_LOG(LogMetaperson2, Warning, TEXT("UAndroidUtils: TakeCameraImage()"));

#if PLATFORM_ANDROID
	if (!CheckPermissions())
	{
		UE_LOG(LogMetaperson2, Warning, TEXT("UAndroidUtils: TakeCameraImage(): Unable to get permissions for camera."));
		return;
	}

	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		DelegateHandle = FJavaWrapper::OnActivityResultDelegate.AddUObject(this, &UAndroidUtils::OnActivityResult);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, UAndroidUtils::TakeCameraImageJMethod);
	}
#endif

}
