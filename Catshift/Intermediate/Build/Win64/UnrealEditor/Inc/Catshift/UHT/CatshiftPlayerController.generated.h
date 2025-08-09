// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "CatshiftPlayerController.h"

#ifdef CATSHIFT_CatshiftPlayerController_generated_h
#error "CatshiftPlayerController.generated.h already included, missing '#pragma once' in CatshiftPlayerController.h"
#endif
#define CATSHIFT_CatshiftPlayerController_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class ACatshiftPlayerController ************************************************
CATSHIFT_API UClass* Z_Construct_UClass_ACatshiftPlayerController_NoRegister();

#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftPlayerController_h_18_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesACatshiftPlayerController(); \
	friend struct Z_Construct_UClass_ACatshiftPlayerController_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend CATSHIFT_API UClass* Z_Construct_UClass_ACatshiftPlayerController_NoRegister(); \
public: \
	DECLARE_CLASS2(ACatshiftPlayerController, APlayerController, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Config), CASTCLASS_None, TEXT("/Script/Catshift"), Z_Construct_UClass_ACatshiftPlayerController_NoRegister) \
	DECLARE_SERIALIZER(ACatshiftPlayerController)


#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftPlayerController_h_18_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API ACatshiftPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	/** Deleted move- and copy-constructors, should never be used */ \
	ACatshiftPlayerController(ACatshiftPlayerController&&) = delete; \
	ACatshiftPlayerController(const ACatshiftPlayerController&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, ACatshiftPlayerController); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(ACatshiftPlayerController); \
	DEFINE_ABSTRACT_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(ACatshiftPlayerController) \
	NO_API virtual ~ACatshiftPlayerController();


#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftPlayerController_h_15_PROLOG
#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftPlayerController_h_18_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftPlayerController_h_18_INCLASS_NO_PURE_DECLS \
	FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftPlayerController_h_18_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class ACatshiftPlayerController;

// ********** End Class ACatshiftPlayerController **************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_CatshiftPlayerController_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
