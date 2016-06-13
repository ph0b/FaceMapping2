Mapping an Intel® RealSense™ SDK Face Scan to a 3D Head Model
======================================================

The face mapping sample uses the Intel RealSense™ 3D Scan module to scan the user's face and then map it onto an existing 3D head model. This is an improvement on a previous code sample sample that used a "stone faced" mapping technique. In this sample the following features have been added:

- Parameterized head. The user can shape the head using a series of sliders. For example, the user can change the width of the head, ears, jaw, and so on.
- Standardized mesh topology. The mapping algorithm can be applied to a head mesh with a standardized topology and can retain the context of each vertex after the mapping is complete. This paves the way for animating the head, improved blending, and post-mapping effects.
- Color and shape post processing. Morph targets and additional color blending can be applied after the mapping stages are complete to customize the final result.
- Hair. There are new hair models created to fit the base head model. A custom algorithm is used to adjust the hair geometry to conform to the user’s chosen head shape.

For detailed information on this sample, please visit:
https://software.intel.com/en-us/articles/mapping-an-intel-realsense-sdk-face-scan-to-a-3d-head-model

The previous version of the code sample is available here:
https://github.com/GameTechDev/FaceMapping

Build Instructions
==================
The facescan2.sln solution should be built with QtCreator using MSVC2013 compiler or higher and Intel RealSense™ SDK 2016 R2 or higher.

Requirements
============
- Windows 8.1 or Windows 10
- QtCreator and Qt 5.5 or higher with MSVC compiler
- Intel® RealSense™ SDK (2016 R2 release) or newer *
- RealSense™ F200 or SR300 Camera*

* These are only required for scanning new faces. The sample includes a single pre-scanned face for experimenting.

The Intel® RealSense™ SDK can be download here:
https://software.intel.com/en-us/intel-realsense-sdk/download


 