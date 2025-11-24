#include "VATToolsBPLibrary.h"

#include "MaterialEditingLibrary.h"
#include "Materials/MaterialExpressionStaticSwitchParameter.h"
#include "Materials/MaterialExpressionSetMaterialAttributes.h"
#include "Materials/MaterialExpressionGetMaterialAttributes.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialExpressionMaterialAttributeLayers.h"
#include "Materials/MaterialExpressionMakeMaterialAttributes.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"

#include "InstancedFoliageActor.h"
#include "Engine/StaticMeshActor.h"

#include "Landscape.h"
#include "LandscapeSplinesComponent.h"
#include "LandscapeSplineControlPoint.h"
#include "Components/SplineComponent.h"
#include <ComponentReregisterContext.h>

UVATToolsBPLibrary::UVATToolsBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UVATToolsBPLibrary::MarkDirty(AActor* Actor)
{
	if (Actor)
	{
		Actor->MarkPackageDirty();
	}
}

EMaterialConversionStatus UVATToolsBPLibrary::AddVATMaterialFunction(UMaterial* Material)
{
	// Convert the Material to Material Attributes
	if (!Material->bUseMaterialAttributes)
	{
		UMaterialExpressionMakeMaterialAttributes* MakeMaterialAttributes = Cast<UMaterialExpressionMakeMaterialAttributes>(UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionMakeMaterialAttributes::StaticClass()));

		// BaseColor
		if (Material->GetEditorOnlyData()->BaseColor.IsConnected())
		{
			MakeMaterialAttributes->BaseColor.Connect(Material->GetEditorOnlyData()->BaseColor.GetTracedInput().OutputIndex, Material->GetEditorOnlyData()->BaseColor.GetTracedInput().Expression);
		}

		// Metallic
		if (Material->GetEditorOnlyData()->Metallic.IsConnected())
		{
			MakeMaterialAttributes->Metallic.Connect(Material->GetEditorOnlyData()->Metallic.GetTracedInput().OutputIndex, Material->GetEditorOnlyData()->Metallic.GetTracedInput().Expression);
		}

		// Specular
		if (Material->GetEditorOnlyData()->Specular.IsConnected())
		{
			MakeMaterialAttributes->Specular.Connect(Material->GetEditorOnlyData()->Specular.GetTracedInput().OutputIndex, Material->GetEditorOnlyData()->Specular.GetTracedInput().Expression);
		}

		// Roughness
		if (Material->GetEditorOnlyData()->Roughness.IsConnected())
		{
			MakeMaterialAttributes->Roughness.Connect(Material->GetEditorOnlyData()->Roughness.GetTracedInput().OutputIndex, Material->GetEditorOnlyData()->Roughness.GetTracedInput().Expression);
		}

		// EmissiveColor
		if (Material->GetEditorOnlyData()->EmissiveColor.IsConnected())
		{
			MakeMaterialAttributes->EmissiveColor.Connect(Material->GetEditorOnlyData()->EmissiveColor.GetTracedInput().OutputIndex, Material->GetEditorOnlyData()->EmissiveColor.GetTracedInput().Expression);
		}

		// Opacity
		if (Material->GetEditorOnlyData()->Opacity.IsConnected())
		{
			MakeMaterialAttributes->Opacity.Connect(Material->GetEditorOnlyData()->Opacity.GetTracedInput().OutputIndex, Material->GetEditorOnlyData()->Opacity.GetTracedInput().Expression);
		}

		// OpacityMask
		if (Material->GetEditorOnlyData()->OpacityMask.IsConnected())
		{
			MakeMaterialAttributes->OpacityMask.Connect(Material->GetEditorOnlyData()->OpacityMask.GetTracedInput().OutputIndex, Material->GetEditorOnlyData()->OpacityMask.GetTracedInput().Expression);
		}

		// Normal
		if (Material->GetEditorOnlyData()->Normal.IsConnected())
		{
			MakeMaterialAttributes->Normal.Connect(Material->GetEditorOnlyData()->Normal.GetTracedInput().OutputIndex, Material->GetEditorOnlyData()->Normal.GetTracedInput().Expression);
		}

		// SubsurfaceColor
		if (Material->GetEditorOnlyData()->SubsurfaceColor.IsConnected())
		{
			MakeMaterialAttributes->SubsurfaceColor.Connect(Material->GetEditorOnlyData()->SubsurfaceColor.GetTracedInput().OutputIndex, Material->GetEditorOnlyData()->SubsurfaceColor.GetTracedInput().Expression);
		}

		// AmbientOcclusion
		if (Material->GetEditorOnlyData()->AmbientOcclusion.IsConnected())
		{
			MakeMaterialAttributes->AmbientOcclusion.Connect(Material->GetEditorOnlyData()->AmbientOcclusion.GetTracedInput().OutputIndex, Material->GetEditorOnlyData()->AmbientOcclusion.GetTracedInput().Expression);
		}

		Material->bUseMaterialAttributes = true;
		Material->GetEditorOnlyData()->MaterialAttributes.Connect(0, MakeMaterialAttributes);
	}

	// Check if a Material Attribute Layers Expression already exists
	TArray<const UMaterialExpressionMaterialAttributeLayers*> MaterialAttributeLayersExpressions;
	Material->GetAllExpressionsOfType(MaterialAttributeLayersExpressions);

	// If a Material Attribute Layers Expression exists, print a warning to manually check the asset. Either the material already contains the VAT tool, it its not compatible
	if (MaterialAttributeLayersExpressions.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Material %s already has a Material Attribute Layers Expression. Please check if the material is setup correctly"), *Material->GetName());

		return EMaterialConversionStatus::WARNING;
	}
	else
	{
		Material->GetEditorOnlyData()->MaterialAttributes.Expression->MaterialExpressionEditorX -= 1000;

		float RootX = Material->GetEditorOnlyData()->MaterialAttributes.Expression->MaterialExpressionEditorX + 1000;
		float RootY = Material->GetEditorOnlyData()->MaterialAttributes.Expression->MaterialExpressionEditorY;

		FExpressionInput LastConnectedNode = Material->GetEditorOnlyData()->MaterialAttributes.GetTracedInput();

		// UseLayers
		UMaterialExpressionStaticSwitchParameter* LayerSwitchParam = Cast<UMaterialExpressionStaticSwitchParameter>(UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionStaticSwitchParameter::StaticClass()));
		LayerSwitchParam->ParameterName = "UseLayers";
		LayerSwitchParam->MaterialExpressionEditorX = RootX - 50;
		LayerSwitchParam->MaterialExpressionEditorY = RootY;

		// SetMaterialAttributes
		UMaterialExpressionSetMaterialAttributes* SetMaterialAttributes = Cast<UMaterialExpressionSetMaterialAttributes>(UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionSetMaterialAttributes::StaticClass()));

		FExpressionInput NormalExpressionInput;
		NormalExpressionInput.InputName = FName("Normal");
		SetMaterialAttributes->Inputs.Add(NormalExpressionInput);

		FExpressionInput WPOExpressionInput;
		WPOExpressionInput.InputName = FName("World Position Offset");
		SetMaterialAttributes->Inputs.Add(WPOExpressionInput);

		FGuid NormalGuid = FGuid("0FA2821A200F4A4AB719B789C1259C64");
		SetMaterialAttributes->AttributeSetTypes.Add(NormalGuid);

		FGuid WPOGuid = FGuid("F905F895D5814314916D24348C40CE9E");
		SetMaterialAttributes->AttributeSetTypes.Add(WPOGuid);

		SetMaterialAttributes->MaterialExpressionEditorX = RootX - 200;
		SetMaterialAttributes->MaterialExpressionEditorY = RootY;

		// GetMaterialAttributes
		UMaterialExpressionGetMaterialAttributes* GetMaterialAttributes = Cast<UMaterialExpressionGetMaterialAttributes>(UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionGetMaterialAttributes::StaticClass()));

		FExpressionOutput NormalExpressionOutput;
		NormalExpressionOutput.OutputName = FName("Normal");
		GetMaterialAttributes->Outputs.Add(NormalExpressionOutput);
		GetMaterialAttributes->AttributeGetTypes.Add(NormalGuid);

		GetMaterialAttributes->MaterialExpressionEditorX = RootX - 800;
		GetMaterialAttributes->MaterialExpressionEditorY = RootY;

		// GetMaterialAttributes Bone Animation
		UMaterialExpressionGetMaterialAttributes* GetMaterialAttributesBA = Cast<UMaterialExpressionGetMaterialAttributes>(UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionGetMaterialAttributes::StaticClass()));

		GetMaterialAttributesBA->Outputs.Add(NormalExpressionOutput);

		FExpressionOutput WPOExpressionOutput;
		WPOExpressionOutput.OutputName = FName("World Position Offset");
		GetMaterialAttributesBA->Outputs.Add(WPOExpressionOutput);

		GetMaterialAttributesBA->AttributeGetTypes.Add(NormalGuid);
		GetMaterialAttributesBA->AttributeGetTypes.Add(WPOGuid);

		GetMaterialAttributesBA->MaterialExpressionEditorX = RootX - 600;
		GetMaterialAttributesBA->MaterialExpressionEditorY = RootY + 400;

		// BlendAngleCorrectedNormals
		UMaterialExpressionMaterialFunctionCall* BlendAngleCorrectedNormals = Cast<UMaterialExpressionMaterialFunctionCall>(UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionMaterialFunctionCall::StaticClass()));

		UObject* MF = FSoftObjectPath("/Engine/Functions/Engine_MaterialFunctions02/Utility/BlendAngleCorrectedNormals.BlendAngleCorrectedNormals").TryLoad();
		if (!MF)
		{
			UE_LOG(LogTemp, Error, TEXT("Unable to find BlendAngleCorrectedNormals"));
			return EMaterialConversionStatus::ERROR;
		}

		UMaterialFunctionInterface* MF_BlendAngleCorrectedNormals = Cast<UMaterialFunctionInterface>(MF);
		BlendAngleCorrectedNormals->SetMaterialFunction(MF_BlendAngleCorrectedNormals);

		BlendAngleCorrectedNormals->MaterialExpressionEditorX = RootX - 400;
		BlendAngleCorrectedNormals->MaterialExpressionEditorY = RootY + 200;

		// Material Attribute Layers
		UMaterialExpressionMaterialAttributeLayers* MaterialAttributeLayers = Cast<UMaterialExpressionMaterialAttributeLayers>(UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionMaterialAttributeLayers::StaticClass()));

		UObject* ML = FSoftObjectPath("/AnimToTexture/Materials/ML_BoneAnimation.ML_BoneAnimation").TryLoad();
		if (!ML)
		{
			UE_LOG(LogTemp, Error, TEXT("Unable to find ML_BoneAnimation"));
			return EMaterialConversionStatus::ERROR;
		}

		UMaterialFunctionInterface* ML_BoneAnimation = Cast<UMaterialFunctionInterface>(ML);
		FMaterialLayersFunctions MaterialLayerFunctions;
		MaterialLayerFunctions.EditorOnly.LayerStates.Add(true);
		MaterialLayerFunctions.EditorOnly.LayerNames.Add(FText::FromString("Background"));
		MaterialLayerFunctions.EditorOnly.LayerGuids.Add(FGuid::NewGuid());
		MaterialLayerFunctions.EditorOnly.RestrictToLayerRelatives.Add(false);
		MaterialLayerFunctions.EditorOnly.RestrictToBlendRelatives.Add(false);
		MaterialLayerFunctions.EditorOnly.LayerLinkStates.Add(EMaterialLayerLinkState::NotFromParent);
		MaterialLayerFunctions.Layers.Add(ML_BoneAnimation);

		MaterialAttributeLayers->DefaultLayers = MaterialLayerFunctions;

		MaterialAttributeLayers->MaterialExpressionEditorX = RootX - 800;
		MaterialAttributeLayers->MaterialExpressionEditorY = RootY + 400;

		// Connect Nodes
		Material->GetEditorOnlyData()->MaterialAttributes.Connect(0, LayerSwitchParam);
		LayerSwitchParam->B.Connect(0, LastConnectedNode.Expression);
		LayerSwitchParam->A.Connect(0, SetMaterialAttributes);
		SetMaterialAttributes->Inputs[0].Connect(0, GetMaterialAttributes);
		SetMaterialAttributes->Inputs[1].Connect(0, BlendAngleCorrectedNormals);
		SetMaterialAttributes->Inputs[2].Connect(2, GetMaterialAttributesBA);
		GetMaterialAttributes->GetInput(0)->Connect(0, LastConnectedNode.Expression);
		BlendAngleCorrectedNormals->GetInput(1)->Connect(1, GetMaterialAttributes);
		BlendAngleCorrectedNormals->GetInput(0)->Connect(1, GetMaterialAttributesBA);
		GetMaterialAttributesBA->GetInput(0)->Connect(0, MaterialAttributeLayers);

		Material->PostEditChange();
		Material->MarkPackageDirty();

		return EMaterialConversionStatus::SUCCESS;
	}
}

void UVATToolsBPLibrary::ForceRecompileMaterial(UMaterialInterface* Material)
{
	Material->PostEditChange();
	Material->MarkPackageDirty();
}
