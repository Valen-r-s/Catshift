// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Catshift/CatshiftGameMode.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeCatshiftGameMode() {}

// ********** Begin Cross Module References ********************************************************
CATSHIFT_API UClass* Z_Construct_UClass_ACatshiftGameMode();
CATSHIFT_API UClass* Z_Construct_UClass_ACatshiftGameMode_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_AGameModeBase();
UPackage* Z_Construct_UPackage__Script_Catshift();
// ********** End Cross Module References **********************************************************

// ********** Begin Class ACatshiftGameMode ********************************************************
void ACatshiftGameMode::StaticRegisterNativesACatshiftGameMode()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_ACatshiftGameMode;
UClass* ACatshiftGameMode::GetPrivateStaticClass()
{
	using TClass = ACatshiftGameMode;
	if (!Z_Registration_Info_UClass_ACatshiftGameMode.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("CatshiftGameMode"),
			Z_Registration_Info_UClass_ACatshiftGameMode.InnerSingleton,
			StaticRegisterNativesACatshiftGameMode,
			sizeof(TClass),
			alignof(TClass),
			TClass::StaticClassFlags,
			TClass::StaticClassCastFlags(),
			TClass::StaticConfigName(),
			(UClass::ClassConstructorType)InternalConstructor<TClass>,
			(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>,
			UOBJECT_CPPCLASS_STATICFUNCTIONS_FORCLASS(TClass),
			&TClass::Super::StaticClass,
			&TClass::WithinClass::StaticClass
		);
	}
	return Z_Registration_Info_UClass_ACatshiftGameMode.InnerSingleton;
}
UClass* Z_Construct_UClass_ACatshiftGameMode_NoRegister()
{
	return ACatshiftGameMode::GetPrivateStaticClass();
}
struct Z_Construct_UClass_ACatshiftGameMode_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n *  Simple GameMode for a third person game\n */" },
#endif
		{ "HideCategories", "Info Rendering MovementReplication Replication Actor Input Movement Collision Rendering HLOD WorldPartition DataLayers Transformation" },
		{ "IncludePath", "CatshiftGameMode.h" },
		{ "ModuleRelativePath", "CatshiftGameMode.h" },
		{ "ShowCategories", "Input|MouseInput Input|TouchInput" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Simple GameMode for a third person game" },
#endif
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<ACatshiftGameMode>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_ACatshiftGameMode_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AGameModeBase,
	(UObject* (*)())Z_Construct_UPackage__Script_Catshift,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_ACatshiftGameMode_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_ACatshiftGameMode_Statics::ClassParams = {
	&ACatshiftGameMode::StaticClass,
	"Game",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x008003ADu,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_ACatshiftGameMode_Statics::Class_MetaDataParams), Z_Construct_UClass_ACatshiftGameMode_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_ACatshiftGameMode()
{
	if (!Z_Registration_Info_UClass_ACatshiftGameMode.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_ACatshiftGameMode.OuterSingleton, Z_Construct_UClass_ACatshiftGameMode_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_ACatshiftGameMode.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(ACatshiftGameMode);
ACatshiftGameMode::~ACatshiftGameMode() {}
// ********** End Class ACatshiftGameMode **********************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftGameMode_h__Script_Catshift_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_ACatshiftGameMode, ACatshiftGameMode::StaticClass, TEXT("ACatshiftGameMode"), &Z_Registration_Info_UClass_ACatshiftGameMode, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(ACatshiftGameMode), 2761420642U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftGameMode_h__Script_Catshift_92017337(TEXT("/Script/Catshift"),
	Z_CompiledInDeferFile_FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftGameMode_h__Script_Catshift_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftGameMode_h__Script_Catshift_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
