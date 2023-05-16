#include "AvatarSDKWebBrowser.h"
//#include "SWebBrowser.h"

void UAvatarSDKWebBrowser::Init()
{
    const FString ProxyObjectName = TEXT("avatarsdk_proxy");
    if (!CallbackProxy) {
        CallbackProxy = NewObject<UAvatarSDKBrowserCallbackProxy>(this, *ProxyObjectName);
    }
    WebBrowserWidget->BindUObject(ProxyObjectName, CallbackProxy);

	FString jscrpt = TEXT("const CLIENT_ID = 'fO3tpBCOGfEWM5CqbKUAeJ3OKiZjA4i8knIbjSMg';"
"const CLIENT_SECRET = 'qEZoR0x0tTIpxjN1S351Xvzyo62cAdVahi3Z7xfOBZtWXyvN1P7hMU54mUiVzo9zRRNEk1VqNX7PlthQfm9iAV0SOM1BTT835MQ7dVBHIvmbHURxBgcvRHxOGGREuIY7';"
"const EDITOR_URL = 'https://metaperson.avatarsdk.com/iframe.html';"

"function onWindowMessage(evt) {"	
    "if (evt.type === 'message') {"
        "if (evt.data?.source === 'metaperson_editor') {"
            "let data = evt.data;"
            "let evtName = data?.eventName;"
            "if (evtName === 'unity_loaded') {"
                "onUnityLoaded(evt, data);"
            "} else if (evtName === 'model_exported') {"               
            "window.ue.avatarsdk_proxy.avatarexportcallback(event.data.url);"
            "}"
        "}"
    "}"
"}"

"function onUnityLoaded(evt, data) {"
    "let authenticationMessage = {"
        "'eventName': 'authenticate',"
        "'clientId': CLIENT_ID,"
        "'clientSecret': CLIENT_SECRET,"
        "'exportTemplateCode': '',"
    "};"
    "evt.source.postMessage(authenticationMessage, '*');"
        "}"
    "window.addEventListener('message', onWindowMessage);"
    );
	ExecuteJavascript(jscrpt);
}

TSharedRef<SWidget> UAvatarSDKWebBrowser::RebuildWidget()
{
	InitialURL = StartUrl;
   
	OnUrlChanged.AddUniqueDynamic(this, &UAvatarSDKWebBrowser::OnUrlChangedHandler);

	auto Result = Super::RebuildWidget();
	
	return Result;
}

void UAvatarSDKWebBrowser::OnUrlChangedHandler(const FText& Text)
{
	Init();
}

 

void UAvatarSDKBrowserCallbackProxy::DeveloperCredentialsCallback(FString JsonResponse)
{
	
}

void UAvatarSDKBrowserCallbackProxy::AvatarExportCallback(FString Url)
{
}
