//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------
#include "CPUT.h"
#include "CPipeline.h"
#include "CDisplacementMapStage.h"
#include "CHeadGeometryStage.h"
#include "CHeadBlendStage.h"
#include "CHairGeometryStage.h"
#include "CPUTModel.h"
#include "CPUTMaterial.h"
#include "CPUTTexture.h"
#include "CPUTMeshDX11.h"
#include "CPUTSoftwareMesh.h"

bool MappingTweaks::operator==(const MappingTweaks &other) const
{
	bool same = true;
#define CMP_EQL(X) {same &= (X == other.X);}
	CMP_EQL(Scale);
	CMP_EQL(DisplaceOffset);
	CMP_EQL(FaceYaw);
	CMP_EQL(FacePitch);
	CMP_EQL(FaceRoll);
	CMP_EQL(BlendColor1);
	CMP_EQL(BlendColor2);
	CMP_EQL(PostBlendAdjust[0]);
	CMP_EQL(PostBlendAdjust[1]);
	CMP_EQL(PostBlendColorize[0]);
	CMP_EQL(PostBlendColorize[1]);
	CMP_EQL(PostBlendMode);
	CMP_EQL(OutputTextureResolution);
	CMP_EQL(MorphTargetEntries);
	CMP_EQL(OtherHeadBlend);
	CMP_EQL(OtherHeadMesh);
	CMP_EQL(OtherHeadTexture);
	CMP_EQL(Flags)
		return same;
}

CPipeline::CPipeline() :
	DisplacementMapStage(NULL),
	HeadGeometryStage(NULL),
	HeadBlendStage(NULL)
{
}

CPipeline::~CPipeline()
{
	SAFE_DELETE(DisplacementMapStage);
	SAFE_DELETE(HeadGeometryStage);
	SAFE_DELETE(HeadBlendStage);
	for (int i = 0; i < (int)HairStages.size(); i++)
		SAFE_DELETE(HairStages[i]);
	HairStages.clear();
}

void CPipeline::executeDisplacementMapStage(SPipelineInput *input) {
	if (DisplacementMapStage == NULL)
	{
		DisplacementMapStage = new CDisplacementMapStage();
	}

	SDisplacementMapStageInput dmInput = {};
	dmInput.FaceModel = input->FaceModel;
	dmInput.RenderParams = input->RenderParams;
	dmInput.FaceYaw = input->Tweaks->FaceYaw;
	dmInput.FacePitch = input->Tweaks->FacePitch;
	dmInput.FaceRoll = input->Tweaks->FaceRoll;
	DisplacementMapStage->Execute(&dmInput);
}

void CPipeline::executeHeadGeometryStage(SPipelineInput *input, CDisplacementMapStageOutput* displacementMapStageOutput) {
	if (HeadGeometryStage == NULL)
	{
		HeadGeometryStage = new CHeadGeometryStage();
	}

	SHeadGeometryStageInput hgInput = {};
	hgInput.DisplacementMap = displacementMapStageOutput->DepthMap->GetSoftwareTexture(false, true);
	hgInput.DisplacementMapInfo = displacementMapStageOutput;
	hgInput.BaseHeadInfo = input->BaseHeadInfo;
	hgInput.Scale = input->Tweaks->Scale;
	hgInput.ZDisplaceOffset = input->Tweaks->DisplaceOffset.z;
	hgInput.MorphTargetEntries = input->Tweaks->MorphTargetEntries;
	hgInput.OtherHeadBlend = input->Tweaks->OtherHeadBlend;
	hgInput.OtherHeadMesh = input->Tweaks->OtherHeadMesh;
	hgInput.Flags = input->Tweaks->Flags;
	HeadGeometryStage->Execute(&hgInput);
}

void CPipeline::executeHeadBlendStage(SPipelineInput *input, CDisplacementMapStageOutput* displacementMapStageOutput, CPUTSoftwareMesh* deformedMesh) {
	if (HeadBlendStage == NULL)
	{
		HeadBlendStage = new CHeadBlendStage();
	}
	SHeadBlendStageInput hbInput = {};
	hbInput.BaseHeadInfo = input->BaseHeadInfo;
	hbInput.DeformedMesh = deformedMesh;
	hbInput.RenderParams = input->RenderParams;

	hbInput.BlendColor1 = input->Tweaks->BlendColor1;
	hbInput.BlendColor2 = input->Tweaks->BlendColor2;
	hbInput.PostBlendAdjust[0] = input->Tweaks->PostBlendAdjust[0];
	hbInput.PostBlendAdjust[1] = input->Tweaks->PostBlendAdjust[1];
	hbInput.PostBlendColorize[0] = input->Tweaks->PostBlendColorize[0];
	hbInput.PostBlendColorize[1] = input->Tweaks->PostBlendColorize[1];
	hbInput.PostBlendMode = input->Tweaks->PostBlendMode;
	hbInput.Flags = input->Tweaks->Flags;
	hbInput.OtherHeadBlend = input->Tweaks->OtherHeadBlend;
	hbInput.OtherHeadTexture = input->Tweaks->OtherHeadTexture;
	hbInput.GeneratedFaceColorMap = displacementMapStageOutput->ColorMap->GetColorResourceView();
	HeadBlendStage->Execute(&hbInput);
}

void CPipeline::executeHairGeometryStage(SPipelineInput *input, CPUTSoftwareMesh* deformedHead) {
	for (int i = 0; i < (int)input->HairInfo.size(); i++)
	{
		if (i >= (int)HairStages.size())
		{
			HairStages.push_back(new CHairGeometryStage());
		}

		SHairGeometryStageInput hairGeomInput;
		hairGeomInput.BaseHead = input->BaseHeadInfo->BaseHeadMesh;
		hairGeomInput.DeformedHead = deformedHead;
		hairGeomInput.Hair = input->HairInfo[i].Mesh;
		hairGeomInput.ClearCachedProjections = false;
		HairStages[i]->Execute(&hairGeomInput);
	}
	int removeStageCount = (int)HairStages.size() - (int)input->HairInfo.size();
	for (int i = 0; i < removeStageCount; i++)
	{
		SAFE_DELETE(HairStages[HairStages.size() - 1]);
		HairStages.pop_back();
	}
}

void CPipeline::Execute(SPipelineInput *input, CPipelineOutput *output)
{
	executeDisplacementMapStage(input);

	executeHeadGeometryStage(input, &DisplacementMapStage->Output);

	executeHeadBlendStage(input, &DisplacementMapStage->Output, &HeadGeometryStage->DeformedMesh);

	mDeformedMesh.CopyFrom(&HeadGeometryStage->DeformedMesh);
	mDeformedMesh.RemoveComponent(eSMComponent_Tex2);

	executeHairGeometryStage(input, &mDeformedMesh);

	if (output != NULL) {
		output->DiffuseTexture = HeadBlendStage->Output.OutputDiffuse;
		output->DeformedMesh = &mDeformedMesh;

        output->LandmarkIdxToMorphedMeshVertIdx = HeadGeometryStage->LandmarkIdxToMorphedMeshVertIdx;

		output->DeformedHairMeshes.clear();
		for (int i = 0; i < (int)HairStages.size(); i++)
		{
			output->DeformedHairMeshes.push_back(&HairStages[i]->DeformedHair);
		}
	}

}
