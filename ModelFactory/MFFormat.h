#pragma once

static const int MF_MAGIC			= 0x4433534D;
static const int MF_HEADER			= 0x00010000;

static const int MF_SUBMESH			= 0x00020000;
static const int MF_VERTEXINDEX		= 0x00020001;
static const int MF_POSITION		= 0x00020002;
static const int MF_NORMAL			= 0x00020003;
static const int MF_TANGENT			= 0x00020004;
static const int MF_UV1				= 0x00020005;
static const int MF_UV2				= 0x00020006;
static const int MF_BONEINDEX		= 0x00020007;
static const int MF_BONEWEIGHT		= 0x00020008;

static const int MF_MESH			= 0x00030000;
static const int MF_MESHTRANSFORM	= 0x00030001;
static const int MF_SKELETON		= 0x00030002;
static const int MF_CLIP			= 0x00030003;
static const int MF_BONESPERVERTEX	= 0x00030004;

static const int MF_MATERILA		= 0x00040000;
static const int MF_DIFFUSEMAP		= 0x00040011;
static const int MF_NORMALMAP		= 0x00040012;
static const int MF_SPECULARMAP		= 0x00040013;