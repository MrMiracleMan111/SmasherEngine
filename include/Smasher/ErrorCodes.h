#pragma once
#include "Smasher/Base.h"

// ==== NO ERROR ====
#define ERROR_NoError 0

// ==== ENGINE EXCEPTIONS ====
#define ERROR_ExpectedHasError 1
#define ERROR_EngineNotInitialized 2
#define ERROR_EngineIsInHeadlessMode 3
#define ERROR_EngineFailedtoInitializeGPU 4

// ==== LAYER EXCEPTIONS ====
#define ERROR_LayerNotFound 100
#define ERROR_LayerDuplicate 101
#define ERROR_LayerEntityNotFound 102
#define ERROR_CannotRemoveBaseLayer 103

// ==== COMPONENT EXCEPTIONS ====
#define ERROR_ComponentInvalid 200
#define ERROR_ComponentDowncastFailed 202
#define ERROR_MissingComponentDependency 203
#define ERROR_SystemNotInitialized 204
#define ERROR_SystemAlreadyInitialized 205

// ==== CAMERA EXCEPTIONS ====
#define ERROR_CameraTargetNotSet 300

// ==== ENTITY EXCEPTIONS ====
#define ERROR_EntityDuplicateComponent 400
#define ERROR_EntityComponentNotFound 401

// ==== EVENT EXCEPTIONS ====
#define ERROR_EventHandleInvalid 500

// ==== RESOURCE EXCEPTIONS ====
#define ERROR_ResourceFailedToLoad 600
#define ERROR_ResourceNotLoaded 601
#define ERROR_ResourceAlreadyExists 602
#define ERROR_ResourceInvalidNumPaths 603
#define ERROR_MaterialHasNoAlbedo 604
#define ERROR_MaterialHasNoNormals 605
#define ERROR_MaterialHasNoSpecular 606

// ==== PHYSICS EXCEPTIONS ====
#define ERROR_Box2DWorldAlreadyCreated 700
#define ERROR_Box2DBodyIdInvalid 701

// ==== OPENGL EXCEPTIONS ====
#define ERROR_GLEWInitFailed 800
#define ERROR_GPUBlockPoolFull 801
#define ERROR_MaxMaterialBindings 802

// ==== JOBS EXCEPTIONS ====
#define ERROR_NotEnoughThreads 900
#define ERROR_NoJobsAvailable 901
#define ERROR_JobHasNoParents 902
#define ERROR_SyncJobCannotDependOnSyncJob 903

