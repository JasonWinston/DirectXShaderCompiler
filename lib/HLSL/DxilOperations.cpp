///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilOperations.cpp                                                        //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implementation of DXIL operation tables.                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/HLSL/DxilOperations.h"
#include "dxc/Support/Global.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;
using std::vector;
using std::string;


namespace hlsl {

using OC = OP::OpCode;
using OCC = OP::OpCodeClass;

//------------------------------------------------------------------------------
//
//  OP class const-static data and related static methods.
//
/* <py>
import hctdb_instrhelp
</py> */
/* <py::lines('OPCODE-OLOADS')>hctdb_instrhelp.get_oloads_props()</py>*/
// OPCODE-OLOADS:BEGIN
const OP::OpCodeProperty OP::m_OpCodeProps[(unsigned)OP::OpCode::NumOpCodes] = {
//   OpCode                       OpCode name,                OpCodeClass                    OpCodeClass name,              void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  // Temporary, indexable, input, output registers                                                                          void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::TempRegLoad,             "TempRegLoad",              OCC::TempRegLoad,              "tempRegLoad",                false,  true,  true, false, false, false,  true,  true, false, Attribute::ReadOnly, },
  {  OC::TempRegStore,            "TempRegStore",             OCC::TempRegStore,             "tempRegStore",               false,  true,  true, false, false, false,  true,  true, false, Attribute::None,     },
  {  OC::MinPrecXRegLoad,         "MinPrecXRegLoad",          OCC::MinPrecXRegLoad,          "minPrecXRegLoad",            false,  true, false, false, false, false,  true, false, false, Attribute::ReadOnly, },
  {  OC::MinPrecXRegStore,        "MinPrecXRegStore",         OCC::MinPrecXRegStore,         "minPrecXRegStore",           false,  true, false, false, false, false,  true, false, false, Attribute::None,     },
  {  OC::LoadInput,               "LoadInput",                OCC::LoadInput,                "loadInput",                  false,  true,  true, false, false, false,  true,  true, false, Attribute::ReadNone, },
  {  OC::StoreOutput,             "StoreOutput",              OCC::StoreOutput,              "storeOutput",                false,  true,  true, false, false, false,  true,  true, false, Attribute::None,     },

  // Unary float                                                                                                            void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::FAbs,                    "FAbs",                     OCC::Unary,                    "unary",                      false,  true,  true,  true, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Saturate,                "Saturate",                 OCC::Unary,                    "unary",                      false,  true,  true,  true, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::IsNaN,                   "IsNaN",                    OCC::IsSpecialFloat,           "isSpecialFloat",             false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::IsInf,                   "IsInf",                    OCC::IsSpecialFloat,           "isSpecialFloat",             false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::IsFinite,                "IsFinite",                 OCC::IsSpecialFloat,           "isSpecialFloat",             false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::IsNormal,                "IsNormal",                 OCC::IsSpecialFloat,           "isSpecialFloat",             false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Cos,                     "Cos",                      OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Sin,                     "Sin",                      OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Tan,                     "Tan",                      OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Acos,                    "Acos",                     OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Asin,                    "Asin",                     OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Atan,                    "Atan",                     OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Hcos,                    "Hcos",                     OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Hsin,                    "Hsin",                     OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Exp,                     "Exp",                      OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Frc,                     "Frc",                      OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Log,                     "Log",                      OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Sqrt,                    "Sqrt",                     OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Rsqrt,                   "Rsqrt",                    OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },

  // Unary float - rounding                                                                                                 void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::Round_ne,                "Round_ne",                 OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Round_ni,                "Round_ni",                 OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Round_pi,                "Round_pi",                 OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Round_z,                 "Round_z",                  OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },

  // Unary int                                                                                                              void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::Bfrev,                   "Bfrev",                    OCC::Unary,                    "unary",                      false, false, false, false, false, false,  true,  true,  true, Attribute::ReadNone, },
  {  OC::Countbits,               "Countbits",                OCC::UnaryBits,                "unaryBits",                  false, false, false, false, false, false,  true,  true,  true, Attribute::ReadNone, },
  {  OC::FirstbitLo,              "FirstbitLo",               OCC::UnaryBits,                "unaryBits",                  false, false, false, false, false, false,  true,  true,  true, Attribute::ReadNone, },
  {  OC::FirstbitHi,              "FirstbitHi",               OCC::UnaryBits,                "unaryBits",                  false, false, false, false, false, false,  true,  true,  true, Attribute::ReadNone, },
  {  OC::FirstbitSHi,             "FirstbitSHi",              OCC::UnaryBits,                "unaryBits",                  false, false, false, false, false, false,  true,  true,  true, Attribute::ReadNone, },

  // Binary float                                                                                                           void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::FMax,                    "FMax",                     OCC::Binary,                   "binary",                     false,  true,  true,  true, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::FMin,                    "FMin",                     OCC::Binary,                   "binary",                     false,  true,  true,  true, false, false, false, false, false, Attribute::ReadNone, },

  // Binary int                                                                                                             void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::IMax,                    "IMax",                     OCC::Binary,                   "binary",                     false, false, false, false, false, false,  true,  true,  true, Attribute::ReadNone, },
  {  OC::IMin,                    "IMin",                     OCC::Binary,                   "binary",                     false, false, false, false, false, false,  true,  true,  true, Attribute::ReadNone, },
  {  OC::UMax,                    "UMax",                     OCC::Binary,                   "binary",                     false, false, false, false, false, false,  true,  true,  true, Attribute::ReadNone, },
  {  OC::UMin,                    "UMin",                     OCC::Binary,                   "binary",                     false, false, false, false, false, false,  true,  true,  true, Attribute::ReadNone, },

  // Binary int with two outputs                                                                                            void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::IMul,                    "IMul",                     OCC::BinaryWithTwoOuts,        "binaryWithTwoOuts",          false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
  {  OC::UMul,                    "UMul",                     OCC::BinaryWithTwoOuts,        "binaryWithTwoOuts",          false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
  {  OC::UDiv,                    "UDiv",                     OCC::BinaryWithTwoOuts,        "binaryWithTwoOuts",          false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },

  // Binary int with carry                                                                                                  void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::IAddc,                   "IAddc",                    OCC::BinaryWithCarry,          "binaryWithCarry",            false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
  {  OC::UAddc,                   "UAddc",                    OCC::BinaryWithCarry,          "binaryWithCarry",            false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
  {  OC::ISubc,                   "ISubc",                    OCC::BinaryWithCarry,          "binaryWithCarry",            false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
  {  OC::USubc,                   "USubc",                    OCC::BinaryWithCarry,          "binaryWithCarry",            false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },

  // Tertiary float                                                                                                         void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::FMad,                    "FMad",                     OCC::Tertiary,                 "tertiary",                   false,  true,  true,  true, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Fma,                     "Fma",                      OCC::Tertiary,                 "tertiary",                   false, false, false,  true, false, false, false, false, false, Attribute::ReadNone, },

  // Tertiary int                                                                                                           void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::IMad,                    "IMad",                     OCC::Tertiary,                 "tertiary",                   false, false, false, false, false, false,  true,  true,  true, Attribute::ReadNone, },
  {  OC::UMad,                    "UMad",                     OCC::Tertiary,                 "tertiary",                   false, false, false, false, false, false,  true,  true,  true, Attribute::ReadNone, },
  {  OC::Msad,                    "Msad",                     OCC::Tertiary,                 "tertiary",                   false, false, false, false, false, false, false,  true,  true, Attribute::ReadNone, },
  {  OC::Ibfe,                    "Ibfe",                     OCC::Tertiary,                 "tertiary",                   false, false, false, false, false, false, false,  true,  true, Attribute::ReadNone, },
  {  OC::Ubfe,                    "Ubfe",                     OCC::Tertiary,                 "tertiary",                   false, false, false, false, false, false, false,  true,  true, Attribute::ReadNone, },

  // Quaternary                                                                                                             void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::Bfi,                     "Bfi",                      OCC::Quaternary,               "quaternary",                 false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },

  // Dot                                                                                                                    void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::Dot2,                    "Dot2",                     OCC::Dot2,                     "dot2",                       false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Dot3,                    "Dot3",                     OCC::Dot3,                     "dot3",                       false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::Dot4,                    "Dot4",                     OCC::Dot4,                     "dot4",                       false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },

  // Resources                                                                                                              void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::CreateHandle,            "CreateHandle",             OCC::CreateHandle,             "createHandle",                true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::CBufferLoad,             "CBufferLoad",              OCC::CBufferLoad,              "cbufferLoad",                false,  true,  true,  true, false,  true,  true,  true,  true, Attribute::ReadOnly, },
  {  OC::CBufferLoadLegacy,       "CBufferLoadLegacy",        OCC::CBufferLoadLegacy,        "cbufferLoadLegacy",          false,  true,  true,  true, false, false,  true,  true, false, Attribute::ReadOnly, },

  // Resources - sample                                                                                                     void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::Sample,                  "Sample",                   OCC::Sample,                   "sample",                     false,  true,  true, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::SampleBias,              "SampleBias",               OCC::SampleBias,               "sampleBias",                 false,  true,  true, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::SampleLevel,             "SampleLevel",              OCC::SampleLevel,              "sampleLevel",                false,  true,  true, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::SampleGrad,              "SampleGrad",               OCC::SampleGrad,               "sampleGrad",                 false,  true,  true, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::SampleCmp,               "SampleCmp",                OCC::SampleCmp,                "sampleCmp",                  false,  true,  true, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::SampleCmpLevelZero,      "SampleCmpLevelZero",       OCC::SampleCmpLevelZero,       "sampleCmpLevelZero",         false,  true,  true, false, false, false, false, false, false, Attribute::ReadOnly, },

  // Resources                                                                                                              void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::TextureLoad,             "TextureLoad",              OCC::TextureLoad,              "textureLoad",                false,  true,  true, false, false, false,  true,  true, false, Attribute::ReadOnly, },
  {  OC::TextureStore,            "TextureStore",             OCC::TextureStore,             "textureStore",               false,  true,  true, false, false, false,  true,  true, false, Attribute::None,     },
  {  OC::BufferLoad,              "BufferLoad",               OCC::BufferLoad,               "bufferLoad",                 false,  true,  true, false, false, false,  true,  true,  true, Attribute::ReadOnly, },
  {  OC::BufferStore,             "BufferStore",              OCC::BufferStore,              "bufferStore",                false,  true,  true, false, false, false,  true,  true,  true, Attribute::None,     },
  {  OC::BufferUpdateCounter,     "BufferUpdateCounter",      OCC::BufferUpdateCounter,      "bufferUpdateCounter",         true, false, false, false, false, false, false, false, false, Attribute::None,     },
  {  OC::CheckAccessFullyMapped,  "CheckAccessFullyMapped",   OCC::CheckAccessFullyMapped,   "checkAccessFullyMapped",     false, false, false, false, false, false, false,  true, false, Attribute::ReadOnly, },
  {  OC::GetDimensions,           "GetDimensions",            OCC::GetDimensions,            "getDimensions",               true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },

  // Resources - gather                                                                                                     void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::TextureGather,           "TextureGather",            OCC::TextureGather,            "textureGather",              false, false,  true, false, false, false, false,  true, false, Attribute::ReadOnly, },
  {  OC::TextureGatherCmp,        "TextureGatherCmp",         OCC::TextureGatherCmp,         "textureGatherCmp",           false, false,  true, false, false, false, false,  true, false, Attribute::ReadOnly, },

  //                                                                                                                        void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::ToDelete5,               "ToDelete5",                OCC::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::None,     },
  {  OC::ToDelete6,               "ToDelete6",                OCC::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::None,     },

  // Resources - sample                                                                                                     void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::Texture2DMSGetSamplePosition, "Texture2DMSGetSamplePosition", OCC::Texture2DMSGetSamplePosition, "texture2DMSGetSamplePosition",   true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::RenderTargetGetSamplePosition, "RenderTargetGetSamplePosition", OCC::RenderTargetGetSamplePosition, "renderTargetGetSamplePosition",   true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::RenderTargetGetSampleCount, "RenderTargetGetSampleCount", OCC::RenderTargetGetSampleCount, "renderTargetGetSampleCount",   true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },

  // Synchronization                                                                                                        void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::AtomicBinOp,             "AtomicBinOp",              OCC::AtomicBinOp,              "atomicBinOp",                false, false, false, false, false, false, false,  true, false, Attribute::None,     },
  {  OC::AtomicCompareExchange,   "AtomicCompareExchange",    OCC::AtomicCompareExchange,    "atomicCompareExchange",      false, false, false, false, false, false, false,  true, false, Attribute::None,     },
  {  OC::Barrier,                 "Barrier",                  OCC::Barrier,                  "barrier",                     true, false, false, false, false, false, false, false, false, Attribute::None,     },

  // Pixel shader                                                                                                           void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::CalculateLOD,            "CalculateLOD",             OCC::CalculateLOD,             "calculateLOD",               false, false,  true, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::Discard,                 "Discard",                  OCC::Discard,                  "discard",                     true, false, false, false, false, false, false, false, false, Attribute::None,     },
  {  OC::DerivCoarseX,            "DerivCoarseX",             OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::DerivCoarseY,            "DerivCoarseY",             OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::DerivFineX,              "DerivFineX",               OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::DerivFineY,              "DerivFineY",               OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::EvalSnapped,             "EvalSnapped",              OCC::EvalSnapped,              "evalSnapped",                false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::EvalSampleIndex,         "EvalSampleIndex",          OCC::EvalSampleIndex,          "evalSampleIndex",            false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::EvalCentroid,            "EvalCentroid",             OCC::EvalCentroid,             "evalCentroid",               false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },

  // Compute shader                                                                                                         void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::ThreadId,                "ThreadId",                 OCC::ThreadId,                 "threadId",                   false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
  {  OC::GroupId,                 "GroupId",                  OCC::GroupId,                  "groupId",                    false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
  {  OC::ThreadIdInGroup,         "ThreadIdInGroup",          OCC::ThreadIdInGroup,          "threadIdInGroup",            false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
  {  OC::FlattenedThreadIdInGroup, "FlattenedThreadIdInGroup", OCC::FlattenedThreadIdInGroup, "flattenedThreadIdInGroup",   false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },

  // Geometry shader                                                                                                        void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::EmitStream,              "EmitStream",               OCC::EmitStream,               "emitStream",                  true, false, false, false, false, false, false, false, false, Attribute::None,     },
  {  OC::CutStream,               "CutStream",                OCC::CutStream,                "cutStream",                   true, false, false, false, false, false, false, false, false, Attribute::None,     },
  {  OC::EmitThenCutStream,       "EmitThenCutStream",        OCC::EmitThenCutStream,        "emitThenCutStream",           true, false, false, false, false, false, false, false, false, Attribute::None,     },

  // Double precision                                                                                                       void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::MakeDouble,              "MakeDouble",               OCC::MakeDouble,               "makeDouble",                 false, false, false,  true, false, false, false, false, false, Attribute::ReadNone, },

  //                                                                                                                        void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::ToDelete1,               "ToDelete1",                OCC::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::None,     },
  {  OC::ToDelete2,               "ToDelete2",                OCC::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::None,     },

  // Double precision                                                                                                       void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::SplitDouble,             "SplitDouble",              OCC::SplitDouble,              "splitDouble",                false, false, false,  true, false, false, false, false, false, Attribute::ReadNone, },

  //                                                                                                                        void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::ToDelete3,               "ToDelete3",                OCC::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::None,     },
  {  OC::ToDelete4,               "ToDelete4",                OCC::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::None,     },

  // Domain and hull shader                                                                                                 void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::LoadOutputControlPoint,  "LoadOutputControlPoint",   OCC::LoadOutputControlPoint,   "loadOutputControlPoint",     false,  true,  true, false, false, false,  true,  true, false, Attribute::ReadNone, },
  {  OC::LoadPatchConstant,       "LoadPatchConstant",        OCC::LoadPatchConstant,        "loadPatchConstant",          false,  true,  true, false, false, false,  true,  true, false, Attribute::ReadNone, },

  // Domain shader                                                                                                          void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::DomainLocation,          "DomainLocation",           OCC::DomainLocation,           "domainLocation",             false, false,  true, false, false, false, false, false, false, Attribute::ReadNone, },

  // Hull shader                                                                                                            void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::StorePatchConstant,      "StorePatchConstant",       OCC::StorePatchConstant,       "storePatchConstant",         false,  true,  true, false, false, false,  true,  true, false, Attribute::None,     },
  {  OC::OutputControlPointID,    "OutputControlPointID",     OCC::OutputControlPointID,     "outputControlPointID",       false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
  {  OC::PrimitiveID,             "PrimitiveID",              OCC::PrimitiveID,              "primitiveID",                false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },

  // Other                                                                                                                  void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::CycleCounterLegacy,      "CycleCounterLegacy",       OCC::CycleCounterLegacy,       "cycleCounterLegacy",          true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },

  // Unary float                                                                                                            void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::Htan,                    "Htan",                     OCC::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::ReadNone, },

  // Wave                                                                                                                   void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::WaveCaptureReserved,     "WaveCaptureReserved",      OCC::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::None,     },
  {  OC::WaveIsFirstLane,         "WaveIsFirstLane",          OCC::WaveIsFirstLane,          "waveIsFirstLane",             true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::WaveGetLaneIndex,        "WaveGetLaneIndex",         OCC::WaveGetLaneIndex,         "waveGetLaneIndex",            true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::WaveGetLaneCount,        "WaveGetLaneCount",         OCC::WaveGetLaneCount,         "waveGetLaneCount",            true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::WaveIsHelperLaneReserved, "WaveIsHelperLaneReserved", OCC::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::None,     },
  {  OC::WaveAnyTrue,             "WaveAnyTrue",              OCC::WaveAnyTrue,              "waveAnyTrue",                 true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::WaveAllTrue,             "WaveAllTrue",              OCC::WaveAllTrue,              "waveAllTrue",                 true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::WaveActiveAllEqual,      "WaveActiveAllEqual",       OCC::WaveActiveAllEqual,       "waveActiveAllEqual",         false,  true,  true,  true,  true,  true,  true,  true,  true, Attribute::ReadOnly, },
  {  OC::WaveActiveBallot,        "WaveActiveBallot",         OCC::WaveActiveBallot,         "waveActiveBallot",            true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::WaveReadLaneAt,          "WaveReadLaneAt",           OCC::WaveReadLaneAt,           "waveReadLaneAt",             false,  true,  true,  true,  true,  true,  true,  true,  true, Attribute::ReadOnly, },
  {  OC::WaveReadLaneFirst,       "WaveReadLaneFirst",        OCC::WaveReadLaneFirst,        "waveReadLaneFirst",          false,  true,  true, false,  true,  true,  true,  true,  true, Attribute::ReadOnly, },
  {  OC::WaveActiveOp,            "WaveActiveOp",             OCC::WaveActiveOp,             "waveActiveOp",               false,  true,  true,  true,  true,  true,  true,  true,  true, Attribute::ReadOnly, },
  {  OC::WaveActiveBit,           "WaveActiveBit",            OCC::WaveActiveBit,            "waveActiveBit",              false, false, false, false, false,  true,  true,  true,  true, Attribute::ReadOnly, },
  {  OC::WavePrefixOp,            "WavePrefixOp",             OCC::WavePrefixOp,             "wavePrefixOp",               false,  true,  true,  true, false,  true,  true,  true,  true, Attribute::ReadOnly, },
  {  OC::WaveGetOrderedIndex,     "WaveGetOrderedIndex",      OCC::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::None,     },

  //                                                                                                                        void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::GlobalOrderedCountIncReserved, "GlobalOrderedCountIncReserved", OCC::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::None,     },

  // Wave                                                                                                                   void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::QuadReadLaneAt,          "QuadReadLaneAt",           OCC::QuadReadLaneAt,           "quadReadLaneAt",             false,  true,  true,  true,  true,  true,  true,  true,  true, Attribute::ReadOnly, },
  {  OC::QuadOp,                  "QuadOp",                   OCC::QuadOp,                   "quadOp",                     false,  true,  true,  true, false,  true,  true,  true,  true, Attribute::ReadOnly, },

  // Bitcasts with different sizes                                                                                          void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::BitcastI16toF16,         "BitcastI16toF16",          OCC::BitcastI16toF16,          "bitcastI16toF16",             true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::BitcastF16toI16,         "BitcastF16toI16",          OCC::BitcastF16toI16,          "bitcastF16toI16",             true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::BitcastI32toF32,         "BitcastI32toF32",          OCC::BitcastI32toF32,          "bitcastI32toF32",             true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::BitcastF32toI32,         "BitcastF32toI32",          OCC::BitcastF32toI32,          "bitcastF32toI32",             true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::BitcastI64toF64,         "BitcastI64toF64",          OCC::BitcastI64toF64,          "bitcastI64toF64",             true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::BitcastF64toI64,         "BitcastF64toI64",          OCC::BitcastF64toI64,          "bitcastF64toI64",             true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },

  // GS                                                                                                                     void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::GSInstanceID,            "GSInstanceID",             OCC::GSInstanceID,             "gsInstanceID",               false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },

  // Legacy floating-point                                                                                                  void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::LegacyF32ToF16,          "LegacyF32ToF16",           OCC::LegacyF32ToF16,           "legacyF32ToF16",              true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::LegacyF16ToF32,          "LegacyF16ToF32",           OCC::LegacyF16ToF32,           "legacyF16ToF32",              true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },

  // Double precision                                                                                                       void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::LegacyDoubleToFloat,     "LegacyDoubleToFloat",      OCC::LegacyDoubleToFloat,      "legacyDoubleToFloat",         true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::LegacyDoubleToSInt32,    "LegacyDoubleToSInt32",     OCC::LegacyDoubleToSInt32,     "legacyDoubleToSInt32",        true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },
  {  OC::LegacyDoubleToUInt32,    "LegacyDoubleToUInt32",     OCC::LegacyDoubleToUInt32,     "legacyDoubleToUInt32",        true, false, false, false, false, false, false, false, false, Attribute::ReadNone, },

  // Wave                                                                                                                   void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::WaveAllBitCount,         "WaveAllBitCount",          OCC::WaveAllOp,                "waveAllOp",                   true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },
  {  OC::WavePrefixBitCount,      "WavePrefixBitCount",       OCC::WavePrefixOp,             "wavePrefixOp",                true, false, false, false, false, false, false, false, false, Attribute::ReadOnly, },

  // Pixel shader                                                                                                           void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
  {  OC::SampleIndex,             "SampleIndex",              OCC::SampleIndex,              "sampleIndex",                false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
  {  OC::Coverage,                "Coverage",                 OCC::Coverage,                 "coverage",                   false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
  {  OC::InnerCoverage,           "InnerCoverage",            OCC::InnerCoverage,            "innerCoverage",              false, false, false, false, false, false, false,  true, false, Attribute::ReadNone, },
};
// OPCODE-OLOADS:END

const char *OP::m_OverloadTypeName[kNumTypeOverloads] = {
  "void", "f16", "f32", "f64", "i1", "i8", "i16", "i32", "i64"
};

const char *OP::m_NamePrefix = "dx.op.";

// Keep sync with DXIL::AtomicBinOpCode
static const char *AtomicBinOpCodeName[] = {
    "AtomicAdd",
    "AtomicAnd",
    "AtomicOr",
    "AtomicXor",
    "AtomicIMin",
    "AtomicIMax",
    "AtomicUMin",
    "AtomicUMax",
    "AtomicExchange",
    "AtomicInvalid"           // Must be last.
};

unsigned OP::GetTypeSlot(Type *pType) {
  Type::TypeID T = pType->getTypeID();
  switch (T) {
  case Type::VoidTyID:    return 0;
  case Type::HalfTyID:    return 1;
  case Type::FloatTyID:   return 2;
  case Type::DoubleTyID:  return 3;
  case Type::IntegerTyID: {
    IntegerType *pIT = dyn_cast<IntegerType>(pType);
    unsigned Bits = pIT->getBitWidth();
    switch (Bits) {
    case 1:               return 4;
    case 8:               return 5;
    case 16:              return 6;
    case 32:              return 7;
    case 64:              return 8;
    }
  }
  }
  return UINT_MAX;
}

const char *OP::GetOverloadTypeName(unsigned TypeSlot) {
  DXASSERT(TypeSlot < kNumTypeOverloads, "otherwise caller passed OOB index");
  return m_OverloadTypeName[TypeSlot];
}

const char *OP::GetOpCodeName(OpCode OpCode) {
  DXASSERT(0 <= (unsigned)OpCode && OpCode < OpCode::NumOpCodes, "otherwise caller passed OOB index");
  return m_OpCodeProps[(unsigned)OpCode].pOpCodeName;
}

const char *OP::GetAtomicOpName(DXIL::AtomicBinOpCode OpCode) {
  unsigned opcode = static_cast<unsigned>(OpCode);
  DXASSERT_LOCALVAR(opcode, opcode < static_cast<unsigned>(DXIL::AtomicBinOpCode::Invalid), "otherwise caller passed OOB index");
  return AtomicBinOpCodeName[static_cast<unsigned>(OpCode)];
}

const OP::OpCodeClass OP::GetOpCodeClass(OpCode OpCode) {
  DXASSERT(0 <= (unsigned)OpCode && OpCode < OpCode::NumOpCodes, "otherwise caller passed OOB index");
  return m_OpCodeProps[(unsigned)OpCode].OpCodeClass;
}

const char *OP::GetOpCodeClassName(OpCode OpCode) {
  DXASSERT(0 <= (unsigned)OpCode && OpCode < OpCode::NumOpCodes, "otherwise caller passed OOB index");
  return m_OpCodeProps[(unsigned)OpCode].pOpCodeClassName;
}

bool OP::IsOverloadLegal(OpCode OpCode, Type *pType) {
  DXASSERT(0 <= (unsigned)OpCode && OpCode < OpCode::NumOpCodes, "otherwise caller passed OOB index");
  unsigned TypeSlot = GetTypeSlot(pType);
  return TypeSlot != UINT_MAX && m_OpCodeProps[(unsigned)OpCode].bAllowOverload[TypeSlot];
}

bool OP::CheckOpCodeTable() {
  for (unsigned i = 0; i < (unsigned)OpCode::NumOpCodes; i++) {
    if ((unsigned)m_OpCodeProps[i].OpCode != i)
      return false;
  }

  return true;
}

bool OP::IsDxilOpFunc(const llvm::Function *F) {
  StringRef name = F->getName();
  return name.startswith(OP::m_NamePrefix);
}

bool OP::IsDxilOpFuncCallInst(const llvm::Instruction *I) {
  const CallInst *CI = dyn_cast<CallInst>(I);
  if (CI == nullptr) return false;
  return IsDxilOpFunc(CI->getCalledFunction());
}

bool OP::IsDxilOpFuncCallInst(const llvm::Instruction *I, OpCode opcode) {
  if (!IsDxilOpFuncCallInst(I)) return false;
  return llvm::cast<llvm::ConstantInt>(I->getOperand(0))->getZExtValue() == (unsigned)opcode;
}

OP::OpCode OP::GetDxilOpFuncCallInst(const llvm::Instruction *I) {
  DXASSERT(IsDxilOpFuncCallInst(I), "else caller didn't call IsDxilOpFuncCallInst to check");
  return (OP::OpCode)llvm::cast<llvm::ConstantInt>(I->getOperand(0))->getZExtValue();
}

bool OP::IsDxilOpWave(OpCode C) {
  unsigned op = (unsigned)C;
  /* <py::lines('OPCODE-WAVE')>hctdb_instrhelp.get_instrs_pred("op", "is_wave")</py>*/
  // OPCODE-WAVE:BEGIN
  // Instructions: WaveCaptureReserved=114, WaveIsFirstLane=115,
  // WaveGetLaneIndex=116, WaveGetLaneCount=117, WaveIsHelperLaneReserved=118,
  // WaveAnyTrue=119, WaveAllTrue=120, WaveActiveAllEqual=121,
  // WaveActiveBallot=122, WaveReadLaneAt=123, WaveReadLaneFirst=124,
  // WaveActiveOp=125, WaveActiveBit=126, WavePrefixOp=127,
  // WaveGetOrderedIndex=128, QuadReadLaneAt=130, QuadOp=131,
  // WaveAllBitCount=144, WavePrefixBitCount=145
  return 114 <= op && op <= 128 || 130 <= op && op <= 131 || 144 <= op && op <= 145;
  // OPCODE-WAVE:END
}

bool OP::IsDxilOpGradient(OpCode C) {
  unsigned op = (unsigned)C;
  /* <py::lines('OPCODE-GRADIENT')>hctdb_instrhelp.get_instrs_pred("op", "is_gradient")</py>*/
  // OPCODE-GRADIENT:BEGIN
  // Instructions: Sample=61, SampleBias=62, SampleCmp=65, TextureGather=74,
  // TextureGatherCmp=75, CalculateLOD=84, DerivCoarseX=86, DerivCoarseY=87,
  // DerivFineX=88, DerivFineY=89
  return 61 <= op && op <= 62 || op == 65 || 74 <= op && op <= 75 || op == 84 || 86 <= op && op <= 89;
  // OPCODE-GRADIENT:END
}

static Type *GetOrCreateStructType(LLVMContext &Ctx, ArrayRef<Type*> types, StringRef Name, Module *pModule) {
  if (StructType *ST = pModule->getTypeByName(Name)) {
    // TODO: validate the exist type match types if needed.
    return ST;
  }
  else
    return StructType::create(Ctx, types, Name);
}

//------------------------------------------------------------------------------
//
//  OP methods.
//
OP::OP(LLVMContext &Ctx, Module *pModule)
: m_Ctx(Ctx)
, m_pModule(pModule) {
  memset(m_pResRetType, 0, sizeof(m_pResRetType));
  memset(m_pCBufferRetType, 0, sizeof(m_pCBufferRetType));
  memset(m_OpCodeClassCache, 0, sizeof(m_OpCodeClassCache));
  static_assert(_countof(OP::m_OpCodeProps) == (size_t)OP::OpCode::NumOpCodes, "forgot to update OP::m_OpCodeProps");

  m_pHandleType = GetOrCreateStructType(m_Ctx, Type::getInt8PtrTy(m_Ctx), "dx.types.Handle", pModule);

  Type *DimsType[4] = { Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx) };
  m_pDimensionsType = GetOrCreateStructType(m_Ctx, DimsType, "dx.types.Dimensions", pModule);

  Type *SamplePosType[2] = { Type::getFloatTy(m_Ctx), Type::getFloatTy(m_Ctx) };
  m_pSamplePosType = GetOrCreateStructType(m_Ctx, SamplePosType, "dx.types.SamplePos", pModule);

  Type *I32cTypes[2] = { Type::getInt32Ty(m_Ctx), Type::getInt1Ty(m_Ctx) };
  m_pBinaryWithCarryType = GetOrCreateStructType(m_Ctx, I32cTypes, "dx.types.i32c", pModule);

  Type *TwoI32Types[2] = { Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx) };
  m_pBinaryWithTwoOutputsType = GetOrCreateStructType(m_Ctx, TwoI32Types, "dx.types.twoi32", pModule);

  Type *SplitDoubleTypes[2] = { Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx) }; // Lo, Hi.
  m_pSplitDoubleType = GetOrCreateStructType(m_Ctx, SplitDoubleTypes, "dx.types.splitdouble", pModule);

  Type *Int4Types[4] = { Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx) }; // HiHi, HiLo, LoHi, LoLo
  m_pInt4Type = GetOrCreateStructType(m_Ctx, Int4Types, "dx.types.fouri32", pModule);
}

Function *OP::GetOpFunc(OpCode OpCode, Type *pOverloadType) {
  DXASSERT(0 <= (unsigned)OpCode && OpCode < OpCode::NumOpCodes, "otherwise caller passed OOB OpCode");
  _Analysis_assume_(0 <= (unsigned)OpCode && OpCode < OpCode::NumOpCodes);
  DXASSERT(IsOverloadLegal(OpCode, pOverloadType), "otherwise the caller requested illegal operation overload (eg HLSL function with unsupported types for mapped intrinsic function)");
  unsigned TypeSlot = GetTypeSlot(pOverloadType);
  Function *&F = m_OpCodeClassCache[(unsigned)m_OpCodeProps[(unsigned)OpCode].OpCodeClass].pOverloads[TypeSlot];
  if (F != nullptr)
    return F;

  vector<Type*> ArgTypes;      // RetType is ArgTypes[0]
  Type *pETy = pOverloadType;
  Type *pRes = GetHandleType();
  Type *pDim = GetDimensionsType();
  Type *pPos = GetSamplePosType();
  Type *pV = Type::getVoidTy(m_Ctx);
  Type *pI1 = Type::getInt1Ty(m_Ctx);
  Type *pI8 = Type::getInt8Ty(m_Ctx);
  Type *pI16 = Type::getInt16Ty(m_Ctx);
  Type *pI32 = Type::getInt32Ty(m_Ctx);
  Type *pPI32 = Type::getInt32PtrTy(m_Ctx); (pPI32); // Currently unused.
  Type *pI64 = Type::getInt64Ty(m_Ctx); (pI64); // Currently unused.
  Type *pF16 = Type::getHalfTy(m_Ctx);
  Type *pF32 = Type::getFloatTy(m_Ctx);
  Type *pPF32 = Type::getFloatPtrTy(m_Ctx);
  Type *pI32C = GetBinaryWithCarryType();
  Type *p2I32 = GetBinaryWithTwoOutputsType();
  Type *pF64 = Type::getDoubleTy(m_Ctx);
  Type *pSDT = GetSplitDoubleType();  // Split double type.
  Type *pI4S = GetInt4Type(); // 4 i32s in a struct.

  std::string funcName = (Twine(OP::m_NamePrefix) + Twine(GetOpCodeClassName(OpCode))).str();
  // Add ret type to the name.
  if (pOverloadType != pV) {
    funcName = Twine(funcName).concat(".").concat(GetOverloadTypeName(TypeSlot)).str();
  } 
  // Try to find exist function with the same name in the module.
  if (Function *existF = m_pModule->getFunction(funcName)) {
    F = existF;
    return F;
  }

#define A(_x) ArgTypes.emplace_back(_x)
#define RRT(_y) A(GetResRetType(_y))
#define CBRT(_y) A(GetCBufferRetType(_y))

/* <py::lines('OPCODE-OLOAD-FUNCS')>hctdb_instrhelp.get_oloads_funcs()</py>*/
  switch (OpCode) {            // return     OpCode
// OPCODE-OLOAD-FUNCS:BEGIN
    // Temporary, indexable, input, output registers
  case OpCode::TempRegLoad:            A(pETy);     A(pI32); A(pI32); break;
  case OpCode::TempRegStore:           A(pV);       A(pI32); A(pI32); A(pETy); break;
  case OpCode::MinPrecXRegLoad:        A(pETy);     A(pI32); A(pPF32);A(pI32); A(pI8);  break;
  case OpCode::MinPrecXRegStore:       A(pV);       A(pI32); A(pPF32);A(pI32); A(pI8);  A(pETy); break;
  case OpCode::LoadInput:              A(pETy);     A(pI32); A(pI32); A(pI32); A(pI8);  A(pI32); break;
  case OpCode::StoreOutput:            A(pV);       A(pI32); A(pI32); A(pI32); A(pI8);  A(pETy); break;

    // Unary float
  case OpCode::FAbs:                   A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Saturate:               A(pETy);     A(pI32); A(pETy); break;
  case OpCode::IsNaN:                  A(pI1);      A(pI32); A(pETy); break;
  case OpCode::IsInf:                  A(pI1);      A(pI32); A(pETy); break;
  case OpCode::IsFinite:               A(pI1);      A(pI32); A(pETy); break;
  case OpCode::IsNormal:               A(pI1);      A(pI32); A(pETy); break;
  case OpCode::Cos:                    A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Sin:                    A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Tan:                    A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Acos:                   A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Asin:                   A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Atan:                   A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Hcos:                   A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Hsin:                   A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Exp:                    A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Frc:                    A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Log:                    A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Sqrt:                   A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Rsqrt:                  A(pETy);     A(pI32); A(pETy); break;

    // Unary float - rounding
  case OpCode::Round_ne:               A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Round_ni:               A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Round_pi:               A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Round_z:                A(pETy);     A(pI32); A(pETy); break;

    // Unary int
  case OpCode::Bfrev:                  A(pETy);     A(pI32); A(pETy); break;
  case OpCode::Countbits:              A(pI32);     A(pI32); A(pETy); break;
  case OpCode::FirstbitLo:             A(pI32);     A(pI32); A(pETy); break;
  case OpCode::FirstbitHi:             A(pI32);     A(pI32); A(pETy); break;
  case OpCode::FirstbitSHi:            A(pI32);     A(pI32); A(pETy); break;

    // Binary float
  case OpCode::FMax:                   A(pETy);     A(pI32); A(pETy); A(pETy); break;
  case OpCode::FMin:                   A(pETy);     A(pI32); A(pETy); A(pETy); break;

    // Binary int
  case OpCode::IMax:                   A(pETy);     A(pI32); A(pETy); A(pETy); break;
  case OpCode::IMin:                   A(pETy);     A(pI32); A(pETy); A(pETy); break;
  case OpCode::UMax:                   A(pETy);     A(pI32); A(pETy); A(pETy); break;
  case OpCode::UMin:                   A(pETy);     A(pI32); A(pETy); A(pETy); break;

    // Binary int with two outputs
  case OpCode::IMul:                   A(p2I32);    A(pI32); A(pETy); A(pETy); break;
  case OpCode::UMul:                   A(p2I32);    A(pI32); A(pETy); A(pETy); break;
  case OpCode::UDiv:                   A(p2I32);    A(pI32); A(pETy); A(pETy); break;

    // Binary int with carry
  case OpCode::IAddc:                  A(pI32C);    A(pI32); A(pETy); A(pETy); break;
  case OpCode::UAddc:                  A(pI32C);    A(pI32); A(pETy); A(pETy); break;
  case OpCode::ISubc:                  A(pI32C);    A(pI32); A(pETy); A(pETy); break;
  case OpCode::USubc:                  A(pI32C);    A(pI32); A(pETy); A(pETy); break;

    // Tertiary float
  case OpCode::FMad:                   A(pETy);     A(pI32); A(pETy); A(pETy); A(pETy); break;
  case OpCode::Fma:                    A(pETy);     A(pI32); A(pETy); A(pETy); A(pETy); break;

    // Tertiary int
  case OpCode::IMad:                   A(pETy);     A(pI32); A(pETy); A(pETy); A(pETy); break;
  case OpCode::UMad:                   A(pETy);     A(pI32); A(pETy); A(pETy); A(pETy); break;
  case OpCode::Msad:                   A(pETy);     A(pI32); A(pETy); A(pETy); A(pETy); break;
  case OpCode::Ibfe:                   A(pETy);     A(pI32); A(pETy); A(pETy); A(pETy); break;
  case OpCode::Ubfe:                   A(pETy);     A(pI32); A(pETy); A(pETy); A(pETy); break;

    // Quaternary
  case OpCode::Bfi:                    A(pETy);     A(pI32); A(pETy); A(pETy); A(pETy); A(pETy); break;

    // Dot
  case OpCode::Dot2:                   A(pETy);     A(pI32); A(pETy); A(pETy); A(pETy); A(pETy); break;
  case OpCode::Dot3:                   A(pETy);     A(pI32); A(pETy); A(pETy); A(pETy); A(pETy); A(pETy); A(pETy); break;
  case OpCode::Dot4:                   A(pETy);     A(pI32); A(pETy); A(pETy); A(pETy); A(pETy); A(pETy); A(pETy); A(pETy); A(pETy); break;

    // Resources
  case OpCode::CreateHandle:           A(pRes);     A(pI32); A(pI8);  A(pI32); A(pI32); A(pI1);  break;
  case OpCode::CBufferLoad:            A(pETy);     A(pI32); A(pRes); A(pI32); A(pI32); break;
  case OpCode::CBufferLoadLegacy:      CBRT(pETy);  A(pI32); A(pRes); A(pI32); break;

    // Resources - sample
  case OpCode::Sample:                 RRT(pETy);   A(pI32); A(pRes); A(pRes); A(pF32); A(pF32); A(pF32); A(pF32); A(pI32); A(pI32); A(pI32); A(pF32); break;
  case OpCode::SampleBias:             RRT(pETy);   A(pI32); A(pRes); A(pRes); A(pF32); A(pF32); A(pF32); A(pF32); A(pI32); A(pI32); A(pI32); A(pF32); A(pF32); break;
  case OpCode::SampleLevel:            RRT(pETy);   A(pI32); A(pRes); A(pRes); A(pF32); A(pF32); A(pF32); A(pF32); A(pI32); A(pI32); A(pI32); A(pF32); break;
  case OpCode::SampleGrad:             RRT(pETy);   A(pI32); A(pRes); A(pRes); A(pF32); A(pF32); A(pF32); A(pF32); A(pI32); A(pI32); A(pI32); A(pF32); A(pF32); A(pF32); A(pF32); A(pF32); A(pF32); A(pF32); break;
  case OpCode::SampleCmp:              RRT(pETy);   A(pI32); A(pRes); A(pRes); A(pF32); A(pF32); A(pF32); A(pF32); A(pI32); A(pI32); A(pI32); A(pF32); A(pF32); break;
  case OpCode::SampleCmpLevelZero:     RRT(pETy);   A(pI32); A(pRes); A(pRes); A(pF32); A(pF32); A(pF32); A(pF32); A(pI32); A(pI32); A(pI32); A(pF32); break;

    // Resources
  case OpCode::TextureLoad:            RRT(pETy);   A(pI32); A(pRes); A(pI32); A(pI32); A(pI32); A(pI32); A(pI32); A(pI32); A(pI32); break;
  case OpCode::TextureStore:           A(pV);       A(pI32); A(pRes); A(pI32); A(pI32); A(pI32); A(pETy); A(pETy); A(pETy); A(pETy); A(pI8);  break;
  case OpCode::BufferLoad:             RRT(pETy);   A(pI32); A(pRes); A(pI32); A(pI32); break;
  case OpCode::BufferStore:            A(pV);       A(pI32); A(pRes); A(pI32); A(pI32); A(pETy); A(pETy); A(pETy); A(pETy); A(pI8);  break;
  case OpCode::BufferUpdateCounter:    A(pI32);     A(pI32); A(pRes); A(pI8);  break;
  case OpCode::CheckAccessFullyMapped: A(pI1);      A(pI32); A(pI32); break;
  case OpCode::GetDimensions:          A(pDim);     A(pI32); A(pRes); A(pI32); break;

    // Resources - gather
  case OpCode::TextureGather:          RRT(pETy);   A(pI32); A(pRes); A(pRes); A(pF32); A(pF32); A(pF32); A(pF32); A(pI32); A(pI32); A(pI32); break;
  case OpCode::TextureGatherCmp:       RRT(pETy);   A(pI32); A(pRes); A(pRes); A(pF32); A(pF32); A(pF32); A(pF32); A(pI32); A(pI32); A(pI32); A(pF32); break;

    // 
  case OpCode::ToDelete5:              A(pV);       A(pI32); break;
  case OpCode::ToDelete6:              A(pV);       A(pI32); break;

    // Resources - sample
  case OpCode::Texture2DMSGetSamplePosition:A(pPos);     A(pI32); A(pRes); A(pI32); break;
  case OpCode::RenderTargetGetSamplePosition:A(pPos);     A(pI32); A(pI32); break;
  case OpCode::RenderTargetGetSampleCount:A(pI32);     A(pI32); break;

    // Synchronization
  case OpCode::AtomicBinOp:            A(pI32);     A(pI32); A(pRes); A(pI32); A(pI32); A(pI32); A(pI32); A(pI32); break;
  case OpCode::AtomicCompareExchange:  A(pI32);     A(pI32); A(pRes); A(pI32); A(pI32); A(pI32); A(pI32); A(pI32); break;
  case OpCode::Barrier:                A(pV);       A(pI32); A(pI32); break;

    // Pixel shader
  case OpCode::CalculateLOD:           A(pF32);     A(pI32); A(pRes); A(pRes); A(pF32); A(pF32); A(pF32); A(pI1);  break;
  case OpCode::Discard:                A(pV);       A(pI32); A(pI1);  break;
  case OpCode::DerivCoarseX:           A(pETy);     A(pI32); A(pETy); break;
  case OpCode::DerivCoarseY:           A(pETy);     A(pI32); A(pETy); break;
  case OpCode::DerivFineX:             A(pETy);     A(pI32); A(pETy); break;
  case OpCode::DerivFineY:             A(pETy);     A(pI32); A(pETy); break;
  case OpCode::EvalSnapped:            A(pETy);     A(pI32); A(pI32); A(pI32); A(pI8);  A(pI32); A(pI32); break;
  case OpCode::EvalSampleIndex:        A(pETy);     A(pI32); A(pI32); A(pI32); A(pI8);  A(pI32); break;
  case OpCode::EvalCentroid:           A(pETy);     A(pI32); A(pI32); A(pI32); A(pI8);  break;

    // Compute shader
  case OpCode::ThreadId:               A(pI32);     A(pI32); A(pI32); break;
  case OpCode::GroupId:                A(pI32);     A(pI32); A(pI32); break;
  case OpCode::ThreadIdInGroup:        A(pI32);     A(pI32); A(pI32); break;
  case OpCode::FlattenedThreadIdInGroup:A(pI32);     A(pI32); break;

    // Geometry shader
  case OpCode::EmitStream:             A(pV);       A(pI32); A(pI8);  break;
  case OpCode::CutStream:              A(pV);       A(pI32); A(pI8);  break;
  case OpCode::EmitThenCutStream:      A(pV);       A(pI32); A(pI8);  break;

    // Double precision
  case OpCode::MakeDouble:             A(pF64);     A(pI32); A(pI32); A(pI32); break;

    // 
  case OpCode::ToDelete1:              A(pV);       A(pI32); break;
  case OpCode::ToDelete2:              A(pV);       A(pI32); break;

    // Double precision
  case OpCode::SplitDouble:            A(pSDT);     A(pI32); A(pF64); break;

    // 
  case OpCode::ToDelete3:              A(pV);       A(pI32); break;
  case OpCode::ToDelete4:              A(pV);       A(pI32); break;

    // Domain and hull shader
  case OpCode::LoadOutputControlPoint: A(pETy);     A(pI32); A(pI32); A(pI32); A(pI8);  A(pI32); break;
  case OpCode::LoadPatchConstant:      A(pETy);     A(pI32); A(pI32); A(pI32); A(pI8);  break;

    // Domain shader
  case OpCode::DomainLocation:         A(pF32);     A(pI32); A(pI8);  break;

    // Hull shader
  case OpCode::StorePatchConstant:     A(pV);       A(pI32); A(pI32); A(pI32); A(pI8);  A(pETy); break;
  case OpCode::OutputControlPointID:   A(pI32);     A(pI32); break;
  case OpCode::PrimitiveID:            A(pI32);     A(pI32); break;

    // Other
  case OpCode::CycleCounterLegacy:     A(p2I32);    A(pI32); break;

    // Unary float
  case OpCode::Htan:                   A(pETy);     A(pI32); A(pETy); break;

    // Wave
  case OpCode::WaveCaptureReserved:    A(pV);       A(pI32); break;
  case OpCode::WaveIsFirstLane:        A(pI1);      A(pI32); break;
  case OpCode::WaveGetLaneIndex:       A(pI32);     A(pI32); break;
  case OpCode::WaveGetLaneCount:       A(pI32);     A(pI32); break;
  case OpCode::WaveIsHelperLaneReserved:A(pV);       A(pI32); break;
  case OpCode::WaveAnyTrue:            A(pI1);      A(pI32); A(pI1);  break;
  case OpCode::WaveAllTrue:            A(pI1);      A(pI32); A(pI1);  break;
  case OpCode::WaveActiveAllEqual:     A(pI1);      A(pI32); A(pETy); break;
  case OpCode::WaveActiveBallot:       A(pI4S);     A(pI32); A(pI1);  break;
  case OpCode::WaveReadLaneAt:         A(pETy);     A(pI32); A(pETy); A(pI32); break;
  case OpCode::WaveReadLaneFirst:      A(pETy);     A(pI32); A(pETy); break;
  case OpCode::WaveActiveOp:           A(pETy);     A(pI32); A(pETy); A(pI8);  A(pI8);  break;
  case OpCode::WaveActiveBit:          A(pETy);     A(pI32); A(pETy); A(pI8);  break;
  case OpCode::WavePrefixOp:           A(pETy);     A(pI32); A(pETy); A(pI8);  A(pI8);  break;
  case OpCode::WaveGetOrderedIndex:    A(pV);       A(pI32); break;

    // 
  case OpCode::GlobalOrderedCountIncReserved:A(pV);       A(pI32); break;

    // Wave
  case OpCode::QuadReadLaneAt:         A(pETy);     A(pI32); A(pETy); A(pI32); break;
  case OpCode::QuadOp:                 A(pETy);     A(pI32); A(pETy); A(pI8);  break;

    // Bitcasts with different sizes
  case OpCode::BitcastI16toF16:        A(pF16);     A(pI32); A(pI16); break;
  case OpCode::BitcastF16toI16:        A(pI16);     A(pI32); A(pF16); break;
  case OpCode::BitcastI32toF32:        A(pF32);     A(pI32); A(pI32); break;
  case OpCode::BitcastF32toI32:        A(pI32);     A(pI32); A(pF32); break;
  case OpCode::BitcastI64toF64:        A(pF64);     A(pI32); A(pI64); break;
  case OpCode::BitcastF64toI64:        A(pI64);     A(pI32); A(pF64); break;

    // GS
  case OpCode::GSInstanceID:           A(pI32);     A(pI32); break;

    // Legacy floating-point
  case OpCode::LegacyF32ToF16:         A(pI32);     A(pI32); A(pF32); break;
  case OpCode::LegacyF16ToF32:         A(pF32);     A(pI32); A(pI32); break;

    // Double precision
  case OpCode::LegacyDoubleToFloat:    A(pF32);     A(pI32); A(pF64); break;
  case OpCode::LegacyDoubleToSInt32:   A(pI32);     A(pI32); A(pF64); break;
  case OpCode::LegacyDoubleToUInt32:   A(pI32);     A(pI32); A(pF64); break;

    // Wave
  case OpCode::WaveAllBitCount:        A(pI32);     A(pI32); A(pI1);  break;
  case OpCode::WavePrefixBitCount:     A(pI32);     A(pI32); A(pI1);  break;

    // Pixel shader
  case OpCode::SampleIndex:            A(pI32);     A(pI32); break;
  case OpCode::Coverage:               A(pI32);     A(pI32); break;
  case OpCode::InnerCoverage:          A(pI32);     A(pI32); break;
  // OPCODE-OLOAD-FUNCS:END
  default: DXASSERT(false, "otherwise unhandled case"); break;
  }
#undef RRT
#undef A

  FunctionType *pFT;
  DXASSERT(ArgTypes.size() > 1, "otherwise forgot to initialize arguments");
  pFT = FunctionType::get(ArgTypes[0], ArrayRef<Type*>(&ArgTypes[1], ArgTypes.size()-1), false);
  if (pOverloadType != pV) {
    F = Function::Create(pFT, GlobalValue::LinkageTypes::ExternalLinkage, 
                         funcName,
                         m_pModule);
  } else {
    F = Function::Create(pFT, GlobalValue::LinkageTypes::ExternalLinkage, 
                         funcName,
                         m_pModule);
  }
  F->setCallingConv(CallingConv::C);
  F->addFnAttr(Attribute::NoUnwind);
  if (m_OpCodeProps[(unsigned)OpCode].FuncAttr != Attribute::None)
    F->addFnAttr(m_OpCodeProps[(unsigned)OpCode].FuncAttr);

  return F;
}

Type *OP::GetHandleType() const {
  return m_pHandleType;
}

Type *OP::GetDimensionsType() const
{
  return m_pDimensionsType;
}

Type *OP::GetSamplePosType() const
{
  return m_pSamplePosType;
}

Type *OP::GetBinaryWithCarryType() const {
  return m_pBinaryWithCarryType;
}

Type *OP::GetBinaryWithTwoOutputsType() const {
  return m_pBinaryWithTwoOutputsType;
}

Type *OP::GetSplitDoubleType() const {
  return m_pSplitDoubleType;
}

Type *OP::GetInt4Type() const {
  return m_pInt4Type;
}

Type *OP::GetResRetType(Type *pOverloadType) {
  unsigned TypeSlot = GetTypeSlot(pOverloadType);

  if (m_pResRetType[TypeSlot] == nullptr) {
    string TypeName("dx.types.ResRet.");
    TypeName += GetOverloadTypeName(TypeSlot);
    Type *FieldTypes[5] = { pOverloadType, pOverloadType, pOverloadType, pOverloadType, Type::getInt32Ty(m_Ctx) };
    m_pResRetType[TypeSlot] = GetOrCreateStructType(m_Ctx, FieldTypes, TypeName, m_pModule);
  }

  return m_pResRetType[TypeSlot];
}

Type *OP::GetCBufferRetType(Type *pOverloadType) {
  unsigned TypeSlot = GetTypeSlot(pOverloadType);

  if (m_pCBufferRetType[TypeSlot] == nullptr) {
    string TypeName("dx.types.CBufRet.");
    TypeName += GetOverloadTypeName(TypeSlot);
    if (!pOverloadType->isDoubleTy()) {
      Type *FieldTypes[4] = { pOverloadType, pOverloadType, pOverloadType, pOverloadType };
      m_pCBufferRetType[TypeSlot] = GetOrCreateStructType(m_Ctx, FieldTypes, TypeName, m_pModule);
    } else {
      Type *FieldTypes[2] = { pOverloadType, pOverloadType };
      m_pCBufferRetType[TypeSlot] = GetOrCreateStructType(m_Ctx, FieldTypes, TypeName, m_pModule);
    }
  }

  return m_pCBufferRetType[TypeSlot];
}


//------------------------------------------------------------------------------
//
//  LLVM utility methods.
//
Constant *OP::GetI1Const(bool v) {
  return Constant::getIntegerValue(IntegerType::get(m_Ctx, 1), APInt(1, v));
}

Constant *OP::GetI8Const(char v) {
  return Constant::getIntegerValue(IntegerType::get(m_Ctx, 8), APInt(8, v));
}

Constant *OP::GetU8Const(unsigned char v) {
  return GetI8Const((char)v);
}

Constant *OP::GetI16Const(int v) {
  return Constant::getIntegerValue(IntegerType::get(m_Ctx, 16), APInt(16, v));
}

Constant *OP::GetU16Const(unsigned v) {
  return GetI16Const((int)v);
}

Constant *OP::GetI32Const(int v) {
  return Constant::getIntegerValue(IntegerType::get(m_Ctx, 32), APInt(32, v));
}

Constant *OP::GetU32Const(unsigned v) {
  return GetI32Const((int)v);
}

Constant *OP::GetU64Const(unsigned long long v) {
 return Constant::getIntegerValue(IntegerType::get(m_Ctx, 64), APInt(64, v));
}

Constant *OP::GetFloatConst(float v) {
  return ConstantFP::get(m_Ctx, APFloat(v));
}

Constant *OP::GetDoubleConst(double v) {
  return ConstantFP::get(m_Ctx, APFloat(v));
}

} // namespace hlsl
