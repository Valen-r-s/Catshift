// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Variant_Combat/CombatLavaFloor.h"

#ifdef CATSHIFT_CombatLavaFloor_generated_h
#error "CombatLavaFloor.generated.h already included, missing '#pragma once' in CombatLavaFloor.h"
#endif
#define CATSHIFT_CombatLavaFloor_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

class AActor;
class UPrimitiveComponent;
struct FHitResult;

// ********** Begin Class ACombatLavaFloor *********************************************************
#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_Variant_Combat_CombatLavaFloor_h_18_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execOnFloorHit);


CATSHIFT_API UClass* Z_Construct_UClass_ACombatLavaFloor_NoRegister();

#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_Variant_Combat_CombatLavaFloor_h_18_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesACombatLavaFloor(); \
	friend struct Z_Construct_UClass_ACombatLavaFloor_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend CATSHIFT_API UClass* Z_Construct_UClass_ACombatLavaFloor_NoRegister(); \
public: \
	DECLARE_CLASS2(ACombatLavaFloor, AActor, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Config), CASTCLASS_None, TEXT("/Script/Catshift"), Z_Construct_UClass_ACombatLavaFloor_NoRegister) \
	DECLARE_SERIALIZER(ACombatLavaFloor)


#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_Variant_Combat_CombatLavaFloor_h_18_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	ACombatLavaFloor(ACombatLavaFloor&&) = delete; \
	ACombatLavaFloor(const ACombatLavaFloor&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, ACombatLavaFloor); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(ACombatLavaFloor); \
	DEFINE_ABSTRACT_DEFAULT_CONSTRUCTOR_CALL(ACombatLavaFloor) \
	NO_API virtual ~ACombatLavaFloor();


#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_Variant_Combat_CombatLavaFloor_h_15_PROLOG
#define FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_Variant_Combat_CombatLavaFloor_h_18_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_Variant_Combat_CombatLavaFloor_h_18_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_Variant_Combat_CombatLavaFloor_h_18_INCLASS_NO_PURE_DECLS \
	FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_Variant_Combat_CombatLavaFloor_h_18_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class ACombatLavaFloor;

// ********** End Class ACombatLavaFloor ***********************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_valen_OneDrive_Documents_GitHub_Catshift_Catshift_Source_Catshift_Variant_Combat_CombatLavaFloor_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
