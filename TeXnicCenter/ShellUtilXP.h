#ifndef GDS_INCLUDE_SHELLUTILXP_H 
#define GDS_INCLUDE_SHELLUTILXP_H 
 
 
/***************************************************************************** 
 *  Shell Utility Functions for Windows XP. 
 * 
 *  by Giovanni Dicanio <giovanni.dicanio@gmail.com> 
 * 
 *  2011, October 13th 
 * 
 * 
 *  Windows XP lacks some useful Shell APIs like SHBindToObject()  
 *  and SHBindToFolderIDListParent(), which are available for Windows 7. 
 * 
 *  This module provides functions to emulate these useful APIs. 
 * 
 *  These functions have a pure C interface, so they don't throw  
 *  C++ exceptions. 
 * 
\***************************************************************************** 
 
 
#include <Windows.h>        /* Win32 Platform SDK header */ 
#include <ShlObj.h>         /* Shell SDK header */ 
#include <specstrings.h>    /* SAL annotations */ 
 
 
/* Header guard for C++ clients - Assume C declarations for C++ */ 
#ifdef __cplusplus 
extern "C" { 
#endif /* __cplusplus */ 
 
 
 
/* 
 * Kind of equivalent to Shell's SHBindToObject(), with NULL IBindCtx parameter. 
 * 
 * Equivalent to ShellUtilXP_BindToObjectEx(), with NULL IBindCtx parameter. 
 */ 
HRESULT __stdcall ShellUtilXP_BindToObject( 
    __in_opt IShellFolder *psf,  
    __in PCUIDLIST_RELATIVE pidl,  
    __in REFIID riid,  
    __deref_out void **ppv 
); 
 
 
/*  
 * Equivalent to Shell's SHBindToObject(). 
 */ 
HRESULT __stdcall ShellUtilXP_BindToObjectEx( 
    __in_opt IShellFolder *psf,  
    __in PCUIDLIST_RELATIVE pidl,  
    __in_opt IBindCtx *pbc, 
    __in REFIID riid,  
    __deref_out void **ppv 
); 
 
 
/*  
 * Equivalent to Shell's SHBindToFolderIDListParent(). 
 * 
 * Same as SHBindToParent(), except you also specify which root to use. 
 */ 
HRESULT __stdcall ShellUtilXP_BindToFolderIDListParent( 
    __in_opt IShellFolder *psfRoot,  
    __in PCUIDLIST_RELATIVE pidl,  
    __in REFIID riid,  
    __deref_out void **ppv,  
    __deref_opt_out PCUITEMID_CHILD *ppidlLast 
); 
 
 
/* 
 * Equivalent to Shell's SHBindToFolderIDListParentEx(). 
 * 
 * Same as SHBindToParent(), except you also specify which root to use. 
 */ 
HRESULT __stdcall ShellUtilXP_BindToFolderIDListParentEx( 
    __in_opt IShellFolder *psfRoot,  
    __in PCUIDLIST_RELATIVE pidl,  
    __in_opt IBindCtx *pbc,  
    __in REFIID riid,  
    __deref_out void **ppv,  
    __deref_opt_out PCUITEMID_CHILD *ppidlLast 
); 
 
 
 
#ifdef __cplusplus 
} 
#endif /* __cplusplus */ 
 
 
#endif /* GDS_INCLUDE_SHELLUTILXP_H */