// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeCatshift_init() {}
	CATSHIFT_API UFunction* Z_Construct_UDelegateFunction_Catshift_OnEnemyDied__DelegateSignature();
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_Catshift;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_Catshift()
	{
		if (!Z_Registration_Info_UPackage__Script_Catshift.OuterSingleton)
		{
			static UObject* (*const SingletonFuncArray[])() = {
				(UObject* (*)())Z_Construct_UDelegateFunction_Catshift_OnEnemyDied__DelegateSignature,
			};
			static const UECodeGen_Private::FPackageParams PackageParams = {
				"/Script/Catshift",
				SingletonFuncArray,
				UE_ARRAY_COUNT(SingletonFuncArray),
				PKG_CompiledIn | 0x00000000,
				0xCE913A7F,
				0xCFFB03E3,
				METADATA_PARAMS(0, nullptr)
			};
			UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_Catshift.OuterSingleton, PackageParams);
		}
		return Z_Registration_Info_UPackage__Script_Catshift.OuterSingleton;
	}
	static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_Catshift(Z_Construct_UPackage__Script_Catshift, TEXT("/Script/Catshift"), Z_Registration_Info_UPackage__Script_Catshift, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0xCE913A7F, 0xCFFB03E3));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
