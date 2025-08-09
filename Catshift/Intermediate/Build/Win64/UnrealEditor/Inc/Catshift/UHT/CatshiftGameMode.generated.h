// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "CatshiftGameMode.h"

#ifdef CATSHIFT_CatshiftGameMode_generated_h
#error "CatshiftGameMode.generated.h already included, missing '#pragma once' in CatshiftGameMode.h"
#endif
#define CATSHIFT_CatshiftGameMode_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class ACatshiftGameMode ********************************************************
CATSHIFT_API UClass* Z_Construct_UClass_ACatshiftGameMode_NoRegister();

#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftGameMode_h_15_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesACatshiftGameMode(); \
	friend struct Z_Construct_UClass_ACatshiftGameMode_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend CATSHIFT_API UClass* Z_Construct_UClass_ACatshiftGameMode_NoRegister(); \
public: \
	DECLARE_CLASS2(ACatshiftGameMode, AGameModeBase, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Transient | CLASS_Config), CASTCLASS_None, TEXT("/Script/Catshift"), Z_Construct_UClass_ACatshiftGameMode_NoRegister) \
	DECLARE_SERIALIZER(ACatshiftGameMode)


#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftGameMode_h_15_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	ACatshiftGameMode(ACatshiftGameMode&&) = delete; \
	ACatshiftGameMode(const ACatshiftGameMode&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, ACatshiftGameMode); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(ACatshiftGameMode); \
	DEFINE_ABSTRACT_DEFAULT_CONSTRUCTOR_CALL(ACatshiftGameMode) \
	NO_API virtual ~ACatshiftGameMode();


#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftGameMode_h_12_PROLOG
#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftGameMode_h_15_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftGameMode_h_15_INCLASS_NO_PURE_DECLS \
	FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftGameMode_h_15_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class ACatshiftGameMode;

// ********** End Class ACatshiftGameMode **********************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftGameMode_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
