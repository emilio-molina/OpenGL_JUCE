/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   blurPassFrag_glsl;
    const int            blurPassFrag_glslSize = 1723;

    extern const char*   brightPassFrag_glsl;
    const int            brightPassFrag_glslSize = 854;

    extern const char*   finalPassFrag_glsl;
    const int            finalPassFrag_glslSize = 345;

    extern const char*   fragmentShader_glsl;
    const int            fragmentShader_glslSize = 3571;

    extern const char*   screenFrag_glsl;
    const int            screenFrag_glslSize = 261;

    extern const char*   screenVert_glsl;
    const int            screenVert_glslSize = 327;

    extern const char*   selectFragment_glsl;
    const int            selectFragment_glslSize = 128;

    extern const char*   selectVertex_glsl;
    const int            selectVertex_glslSize = 285;

    extern const char*   vertexShader_glsl;
    const int            vertexShader_glslSize = 863;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 9;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
