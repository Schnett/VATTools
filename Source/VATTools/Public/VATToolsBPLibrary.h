// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "VATToolsBPLibrary.generated.h"

UENUM(BlueprintType)
enum class EMaterialConversionStatus : uint8
{
	ERROR, WARNING, SUCCESS
};

UCLASS()
class UVATToolsBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category = "VATTools")
	static void MarkDirty(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "VATTools")
	static EMaterialConversionStatus AddVATMaterialFunction(UMaterial* Material);

	UFUNCTION(BlueprintCallable, Category = "VATTools")
	static void ForceRecompileMaterial(UMaterialInterface* Material);
};

