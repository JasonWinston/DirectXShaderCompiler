//===--- HlslTypes.h  - Type system for HLSL                 ----*- C++ -*-===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HlslTypes.h                                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// Licensed under the MIT license. See COPYRIGHT in the project root for     //
// full license information.                                                 //
//                                                                           //
///
/// \file                                                                    //
/// \brief Defines the HLSL type system interface.                           //
///
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef LLVM_CLANG_AST_HLSLTYPES_H
#define LLVM_CLANG_AST_HLSLTYPES_H

#include "clang/AST/Type.h"             // needs QualType
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/Specifiers.h"
#include "llvm/Support/Casting.h"
#include "dxc/HLSL/DxilConstants.h"

namespace clang {
  class ASTContext;
  class AttributeList;
  class CXXMethodDecl;
  class CXXRecordDecl;
  class ClassTemplateDecl;
  class ExtVectorType;
  class FunctionDecl;
  class FunctionTemplateDecl;
  class InheritableAttr;
  class NamedDecl;
  class TypeSourceInfo;
  class TypedefDecl;
}

namespace hlsl {

/// <summary>Initializes the specified context to support HLSL compilation.</summary>
void InitializeASTContextForHLSL(clang::ASTContext& context);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Type system enumerations.

/// <summary>Scalar types for HLSL identified by a single keyword.</summary>
enum HLSLScalarType {
  HLSLScalarType_unknown,
  HLSLScalarType_bool,
  HLSLScalarType_int,
  HLSLScalarType_uint,
  HLSLScalarType_dword,
  HLSLScalarType_half,
  HLSLScalarType_float,
  HLSLScalarType_double,
  HLSLScalarType_float_min10,
  HLSLScalarType_float_min16,
  HLSLScalarType_int_min12,
  HLSLScalarType_int_min16,
  HLSLScalarType_uint_min16,
  HLSLScalarType_float_lit,
  HLSLScalarType_int_lit,
  HLSLScalarType_int64,
  HLSLScalarType_uint64,
};

static const HLSLScalarType HLSLScalarType_minvalid = HLSLScalarType_bool;
static const HLSLScalarType HLSLScalarType_max = HLSLScalarType_uint64;
static const size_t HLSLScalarTypeCount = static_cast<size_t>(HLSLScalarType_max) + 1;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Type annotations and descriptors.

struct MatrixMemberAccessPositions {
  uint32_t IsValid: 1;  // Whether the member access is valid.
  uint32_t Count  : 3;  // Count of row/col pairs.
  uint32_t R0_Row : 2;  // Zero-based row index for first position.
  uint32_t R0_Col : 2;  // Zero-based column index for first position.
  uint32_t R1_Row : 2;  // ...
  uint32_t R1_Col : 2;
  uint32_t R2_Row : 2;
  uint32_t R2_Col : 2;
  uint32_t R3_Row : 2;
  uint32_t R3_Col : 2;

  bool ContainsDuplicateElements() const
  {
    return
      IsValid && (
       (Count > 1 && ((R1_Row == R0_Row && R1_Col == R0_Col)))   ||
       (Count > 2 && ((R2_Row == R0_Row && R2_Col == R0_Col) ||
                      (R2_Row == R1_Row && R2_Col == R1_Col)))   ||
       (Count > 3 && ((R3_Row == R0_Row && R3_Col == R0_Col) ||
                      (R3_Row == R1_Row && R3_Col == R1_Col) ||
                      (R3_Row == R2_Row && R3_Col == R2_Col)))   );
  }

  void GetPosition(uint32_t index, _Out_ uint32_t* row, _Out_ uint32_t* col) const
  {
    assert(index < 4);
    switch (index)
    {
    case 0: *row = R0_Row; *col = R0_Col; break;
    case 1: *row = R1_Row; *col = R1_Col; break;
    case 2: *row = R2_Row; *col = R2_Col; break;
    default:
    case 3: *row = R3_Row; *col = R3_Col; break;
    }
    assert(0 <= *row && *row <= 3);
    assert(0 <= *col && *col <= 3);
  }

  void SetPosition(uint32_t index, uint32_t row, uint32_t col)
  {
    assert(index < 4);
    assert(0 <= row && row <= 3);
    assert(0 <= col && col <= 3);
    switch (index)
    {
    case 0: R0_Row = row; R0_Col = col; break;
    case 1: R1_Row = row; R1_Col = col; break;
    case 2: R2_Row = row; R2_Col = col; break;
    default:
    case 3: R3_Row = row; R3_Col = col; break;
    }
  }
};

struct VectorMemberAccessPositions {
  uint32_t IsValid: 1;  // Whether the member access is valid.
  uint32_t Count  : 3;  // Count of swizzle components.
  uint32_t Swz0 : 2;    // Zero-based swizzle index for first position.
  uint32_t Swz1 : 2;
  uint32_t Swz2 : 2;
  uint32_t Swz3 : 2;

  bool ContainsDuplicateElements() const
  {
    return
      IsValid && (
       (Count > 1 && (Swz1 == Swz0))    ||
       (Count > 2 && ((Swz2 == Swz0)  ||
                      (Swz2 == Swz1)))  ||
       (Count > 3 && ((Swz3 == Swz0)  ||
                      (Swz3 == Swz1)  ||
                      (Swz3 == Swz2)))   );
  }

  void GetPosition(uint32_t index, _Out_ uint32_t* col) const
  {
    assert(index < 4);
    switch (index)
    {
    case 0: *col = Swz0; break;
    case 1: *col = Swz1; break;
    case 2: *col = Swz2; break;
    default:
    case 3: *col = Swz3; break;
    }
    assert(0 <= *col && *col <= 3);
  }

  void SetPosition(uint32_t index, uint32_t col)
  {
    assert(index < 4);
    assert(0 <= col && col <= 3);
    switch (index)
    {
    case 0: Swz0 = col; break;
    case 1: Swz1 = col; break;
    case 2: Swz2 = col; break;
    default:
    case 3: Swz3 = col; break;
    }
  }
};

/// <summary>Base class for annotations that are rarely used.</summary>
struct UnusualAnnotation {
public:
  enum UnusualAnnotationKind {
    UA_RegisterAssignment,
    UA_ConstantPacking,
    UA_SemanticDecl
  };
private:
  const UnusualAnnotationKind Kind;
public:
  UnusualAnnotation(UnusualAnnotationKind kind) : Kind(kind), Loc() { }
  UnusualAnnotation(UnusualAnnotationKind kind, clang::SourceLocation loc) : Kind(kind), Loc(loc) { }
  UnusualAnnotationKind getKind() const { return Kind; }

  UnusualAnnotation* CopyToASTContext(clang::ASTContext& Context);
  static llvm::ArrayRef<UnusualAnnotation*> CopyToASTContextArray(
    clang::ASTContext& Context, UnusualAnnotation** begin, size_t count);

  /// <summary>Location where the annotation was parsed.</summary>
  clang::SourceLocation Loc;
};

/// <summary>Use this structure to capture a ': register' definition.</summary>
struct RegisterAssignment : public UnusualAnnotation
{
  /// <summary>Initializes a new RegisterAssignment in invalid state.</summary>
  RegisterAssignment() : UnusualAnnotation(UA_RegisterAssignment),
    ShaderProfile(),
    RegisterType(0), RegisterNumber(0), RegisterSpace(0), RegisterOffset(0), IsValid(false)
  {
  }

  RegisterAssignment(const RegisterAssignment& other) : UnusualAnnotation(UA_RegisterAssignment, other.Loc),
    ShaderProfile(other.ShaderProfile),
    RegisterType(other.RegisterType),
    RegisterNumber(other.RegisterNumber),
    RegisterOffset(other.RegisterOffset),
    RegisterSpace(other.RegisterSpace),
    IsValid(other.IsValid)
  {
  }

  llvm::StringRef   ShaderProfile;
  bool              IsValid;
  char              RegisterType;
  uint32_t          RegisterNumber;
  uint32_t          RegisterSpace;
  uint32_t          RegisterOffset;

  void setIsValid(bool value) {
    IsValid = value;
  }

  static bool classof(const UnusualAnnotation *UA) {
    return UA->getKind() == UA_RegisterAssignment;
  }
};

/// <summary>Use this structure to capture a ': packoffset' definition.</summary>
struct ConstantPacking : public UnusualAnnotation
{
  /// <summary>Initializes a new ConstantPacking in invalid state.</summary>
  ConstantPacking() : UnusualAnnotation(UA_ConstantPacking),
    Subcomponent(0), ComponentOffset(0), IsValid(0) { }

  uint32_t Subcomponent;        // Subcomponent specified.
  unsigned ComponentOffset : 2; // 0-3 for the offset specified.
  unsigned IsValid : 1;         // Whether the declaration is valid.

  void setIsValid(bool value) {
    IsValid = value ? 1 : 0;
  }

  static bool classof(const UnusualAnnotation *UA) {
    return UA->getKind() == UA_ConstantPacking;
  }
};

/// <summary>Use this structure to capture a ': SEMANTIC' definition.</summary>
struct SemanticDecl : public UnusualAnnotation
{
  /// <summary>Initializes a new SemanticDecl in invalid state.</summary>
  SemanticDecl() : UnusualAnnotation(UA_SemanticDecl), SemanticName() { }

  /// <summary>Initializes a new SemanticDecl with the specified name.</summary>
  SemanticDecl(llvm::StringRef name) : UnusualAnnotation(UA_SemanticDecl), 
    SemanticName(name) { }

  /// <summary>Name for semantic.</summary>
  llvm::StringRef SemanticName;

  static bool classof(const UnusualAnnotation *UA) {
    return UA->getKind() == UA_SemanticDecl;
  }
};

/// Returns a ParameterModifier initialized as per the attribute list.
ParameterModifier
ParamModFromAttributeList(_In_opt_ clang::AttributeList *pAttributes);

/// Returns a ParameterModifier initialized as per the attribute list.
ParameterModifier
ParamModFromAttrs(llvm::ArrayRef<clang::InheritableAttr *> attributes);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AST manipulation functions.

void AddHLSLMatrixTemplate(
  clang::ASTContext& context,
  _In_ clang::ClassTemplateDecl* vectorTemplateDecl,
  _Outptr_ clang::ClassTemplateDecl** matrixTemplateDecl);

void AddHLSLVectorTemplate(
  clang::ASTContext& context, 
  _Outptr_ clang::ClassTemplateDecl** vectorTemplateDecl);

void AddRecordTypeWithHandle(
            clang::ASTContext& context,
  _Outptr_  clang::CXXRecordDecl** typeDecl, 
  _In_z_    const char* typeName);

/// <summary>Adds the implementation for std::is_equal.</summary>
void AddStdIsEqualImplementation(clang::ASTContext& context, clang::Sema& sema);

/// <summary>
/// Adds a new template type in the specified context with the given name. The record type will have a handle field.
/// </summary>
/// <parm name="context">AST context to which template will be added.</param>
/// <parm name="typeDecl">After execution, template declaration.</param>
/// <parm name="recordDecl">After execution, record declaration for template.</param>
/// <parm name="typeName">Name of template to create.</param>
/// <parm name="templateArgCount">Number of template arguments (one or two).</param>
/// <parm name="defaultTypeArgValue">If assigned, the default argument for the element template.</param>
void AddTemplateTypeWithHandle(
            clang::ASTContext& context,
  _Outptr_  clang::ClassTemplateDecl** typeDecl,
  _Outptr_  clang::CXXRecordDecl** recordDecl,
  _In_z_    const char* typeName,
            uint8_t templateArgCount,
  _In_opt_  clang::TypeSourceInfo* defaultTypeArgValue);

/// <summary>Create a function template declaration for the specified method.</summary>
/// <param name="context">AST context in which to work.</param>
/// <param name="recordDecl">Class in which the function template is declared.</param>
/// <param name="functionDecl">Function for which a template is created.</params>
/// <param name="templateParamNamedDecls">Declarations for templates to the function.</param>
/// <param name="templateParamNamedDeclsCount">Count of template declarations.</param>
/// <returns>A new function template declaration already declared in the class scope.</returns>
clang::FunctionTemplateDecl* CreateFunctionTemplateDecl(
  clang::ASTContext& context,
  _In_ clang::CXXRecordDecl* recordDecl,
  _In_ clang::CXXMethodDecl* functionDecl,
  _In_count_(templateParamNamedDeclsCount) clang::NamedDecl** templateParamNamedDecls,
  size_t templateParamNamedDeclsCount);

clang::TypedefDecl* CreateMatrixSpecializationShorthand(
  clang::ASTContext& context,
  clang::QualType matrixSpecialization,
  HLSLScalarType scalarType,
  size_t rowCount,
  size_t colCount);

clang::TypedefDecl* CreateVectorSpecializationShorthand(
  clang::ASTContext& context,
  clang::QualType vectorSpecialization,
  HLSLScalarType scalarType,
  size_t colCount);

const clang::ExtVectorType *
ConvertHLSLVecMatTypeToExtVectorType(const clang::ASTContext &,
                                     clang::QualType);
bool IsHLSLVecMatType(clang::QualType);
bool IsHLSLVecType(clang::QualType type);
bool IsHLSLMatType(clang::QualType type);
bool IsHLSLInputPatchType(clang::QualType type);
bool IsHLSLOutputPatchType(clang::QualType type);
bool IsHLSLPointStreamType(clang::QualType type);
bool IsHLSLLineStreamType(clang::QualType type);
bool IsHLSLTriangleStreamType(clang::QualType type);
bool IsHLSLStreamOutputType(clang::QualType type);
bool IsHLSLResouceType(clang::QualType type);
clang::QualType GetHLSLResourceResultType(clang::QualType type);
bool IsIncompleteHLSLResourceArrayType(clang::ASTContext& context, clang::QualType type);
clang::QualType GetHLSLInputPatchElementType(clang::QualType type);
unsigned GetHLSLInputPatchCount(clang::QualType type);
clang::QualType GetHLSLOutputPatchElementType(clang::QualType type);
unsigned GetHLSLOutputPatchCount(clang::QualType type);

void GetRowsAndColsForAny(clang::QualType type, uint32_t &rowCount,
                          uint32_t &colCount);
uint32_t GetElementCount(clang::QualType type);
uint32_t GetArraySize(clang::QualType type);
uint32_t GetHLSLVecSize(clang::QualType type);
void GetRowsAndCols(clang::QualType type, uint32_t &rowCount,
                    uint32_t &colCount);
void GetHLSLMatRowColCount(clang::QualType type, uint32_t &row, uint32_t &col);
clang::QualType GetHLSLMatElementType(clang::QualType type);
clang::QualType GetHLSLVecElementType(clang::QualType type);
bool IsIntrinsicOp(const clang::FunctionDecl *FD);
bool GetIntrinsicOp(const clang::FunctionDecl *FD, unsigned &opcode,
                    llvm::StringRef &group);

/// <summary>Adds a function declaration to the specified class record.</summary>
/// <param name="context">ASTContext that owns declarations.</param>
/// <param name="recordDecl">Record declaration in which to add function.</param>
/// <param name="resultType">Result type for function.</param>
/// <param name="paramTypes">Types for function parameters.</param>
/// <param name="paramNames">Names for function parameters.</param>
/// <param name="declarationName">Name for function.</param>
/// <param name="isConst">Whether the function is a const function.</param>
/// <returns>The method declaration for the function.</returns>
clang::CXXMethodDecl* CreateObjectFunctionDeclarationWithParams(
  clang::ASTContext& context,
  _In_ clang::CXXRecordDecl* recordDecl,
  clang::QualType resultType,
  llvm::ArrayRef<clang::QualType> paramTypes,
  llvm::ArrayRef<clang::StringRef> paramNames,
  clang::DeclarationName declarationName,
  bool isConst);

DXIL::ResourceClass GetResourceClassForType(const clang::ASTContext &context,
                                            clang::QualType Ty);

_Success_(return != false)
bool TryParseMatrixShorthand(
  _In_count_(typeNameLen)
            const char* typeName,
            size_t typeNameLen,
  _Out_     HLSLScalarType* parsedType,
  _Out_     int* rowCount,
  _Out_     int* colCount);

_Success_(return != false)
bool TryParseVectorShorthand(
  _In_count_(typeNameLen)
            const char* typeName,
            size_t typeNameLen,
  _Out_     HLSLScalarType* parsedType,
  _Out_     int* elementCount);

}

#endif