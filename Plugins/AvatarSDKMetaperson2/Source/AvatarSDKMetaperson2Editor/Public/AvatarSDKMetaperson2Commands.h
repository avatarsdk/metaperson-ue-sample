/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
* You may not use this file except in compliance with an authorized license
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
* See the License for the specific language governing permissions and limitations under the License.
* Written by Itseez3D, Inc. <support@avatarsdk.com>, December 2024
*/

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "AvatarSDK2MetapersonStyle.h"

class FAvatarSDKMetaperson2Commands : public TCommands<FAvatarSDKMetaperson2Commands>
{
public:

	FAvatarSDKMetaperson2Commands()
		: TCommands<FAvatarSDKMetaperson2Commands>(TEXT("AvatarSDKMetaperson2"), NSLOCTEXT("Contexts", "AvatarSDKMetaperson2", "Avatar SDK Metaperson Plugin"), NAME_None, FAvatarSDK2MetapersonStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};