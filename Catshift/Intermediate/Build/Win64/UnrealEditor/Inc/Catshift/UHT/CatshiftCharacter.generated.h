// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "CatshiftCharacter.h"

#ifdef CATSHIFT_CatshiftCharacter_generated_h
#error "CatshiftCharacter.generated.h already included, missing '#pragma once' in CatshiftCharacter.h"
#endif
#define CATSHIFT_CatshiftCharacter_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class ACatshiftCharacter *******************************************************
#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftCharacter_h_24_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execDoJumpEnd); \
	DECLARE_FUNCTION(execDoJumpStart); \
	DECLARE_FUNCTION(execDoLook); \
	DECLARE_FUNCTION(execDoMove);


CATSHIFT_API UClass* Z_Construct_UClass_ACatshiftCharacter_NoRegister();

#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftCharacter_h_24_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesACatshiftCharacter(); \
	friend struct Z_Construct_UClass_ACatshiftCharacter_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend CATSHIFT_API UClass* Z_Construct_UClass_ACatshiftCharacter_NoRegister(); \
public: \
	DECLARE_CLASS2(ACatshiftCharacter, ACharacter, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Config), CASTCLASS_None, TEXT("/Script/Catshift"), Z_Construct_UClass_ACatshiftCharacter_NoRegister) \
	DECLARE_SERIALIZER(ACatshiftCharacter)


#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftCharacter_h_24_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	ACatshiftCharacter(ACatshiftCharacter&&) = delete; \
	ACatshiftCharacter(const ACatshiftCharacter&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, ACatshiftCharacter); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(ACatshiftCharacter); \
	DEFINE_ABSTRACT_DEFAULT_CONSTRUCTOR_CALL(ACatshiftCharacter) \
	NO_API virtual ~ACatshiftCharacter();


#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftCharacter_h_21_PROLOG
#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftCharacter_h_24_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftCharacter_h_24_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftCharacter_h_24_INCLASS_NO_PURE_DECLS \
	FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftCharacter_h_24_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class ACatshiftCharacter;

// ********** End Class ACatshiftCharacter *********************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftCharacter_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
