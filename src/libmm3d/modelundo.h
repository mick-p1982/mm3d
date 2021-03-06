/*  MM3D Misfit/Maverick Model 3D
 *
 * Copyright (c)2004-2007 Kevin Worcester
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place-Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * See the COPYING file for full license text.
 */


#ifndef __MODELUNDO_H
#define __MODELUNDO_H

#include "mm3dtypes.h"
#include "undo.h"
#include "model.h"
#include "glmath.h"
#include "sorted_list.h"

class ModelUndo : public Undo
{
	public:

		ModelUndo(){ s_allocated++; };
		virtual ~ModelUndo(){ s_allocated--; };
		
		virtual void undo(Model *)= 0;
		virtual void redo(Model *)= 0;

		static int s_allocated;
};

class MU_TranslateSelected : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size(){ return sizeof(MU_TranslateSelected); };

		//void setMatrix(const Matrix &rhs){ m_matrix = rhs; }
		//Matrix getMatrix()const{ return m_matrix; };
		void setVector(const double vec[3])
		{
			memcpy(m_vec,vec,sizeof(m_vec));
		}

	private:
		//Matrix m_matrix;
		double m_vec[3];
};

class MU_RotateSelected : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size(){ return sizeof(MU_RotateSelected); };

		void setMatrixPoint(const Matrix &rhs, const double point[3]);
		Matrix getMatrix()const { return m_matrix; };

	private:

		Matrix m_matrix;
		double m_point[3];
};

class MU_ApplyMatrix : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size(){ return sizeof(MU_ApplyMatrix); };

		void setMatrix(const Matrix &mat,Model::OperationScopeE scope);
		Matrix getMatrix()const { return m_matrix; };

	private:
		Matrix m_matrix;
		Model::OperationScopeE m_scope;
};

class MU_SelectionMode : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setSelectionMode(Model::SelectionModeE mode,Model::SelectionModeE oldMode){ m_mode = mode; m_oldMode = oldMode; };

	private:
		Model::SelectionModeE m_mode;
		Model::SelectionModeE m_oldMode;
};

class MU_Select : public ModelUndo
{
	public:
		MU_Select(Model::SelectionModeE mode):m_mode(mode)
		{}

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		Model::SelectionModeE getSelectionMode()const { return m_mode; };

		void setSelectionDifference(int number,bool selected,bool oldSelected);
		void toggle(int number);

		unsigned diffCount(){ return m_diff.size(); };

	private:
		typedef struct _SelectionDifference_t
		{
			int  number;
			bool selected;
			bool oldSelected;
			bool operator<(const struct _SelectionDifference_t &rhs)const
			{
				return (this->number<rhs.number);
			}
			bool operator== (const struct _SelectionDifference_t &rhs)const
			{
				return (this->number==rhs.number);
			}
		} SelectionDifferenceT;

		//2019: setSelectionDifference used this. It seems nuts/prevents
		//combining. 
		//typedef sorted_list<SelectionDifferenceT> SelectionDifferenceList;
		typedef std::vector<SelectionDifferenceT> SelectionDifferenceList;

		Model::SelectionModeE m_mode;
		SelectionDifferenceList m_diff;
		
};

class MU_SetSelectedUv : public ModelUndo
{
	public:
		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setSelectedUv(const std::vector<int> &newUv, const std::vector<int> &oldUv);

	private:
		std::vector<int> m_oldUv;
		std::vector<int> m_newUv;
};

class MU_Hide : public ModelUndo
{
	public:
		MU_Hide(Model::SelectionModeE mode):m_mode(mode)
		{}

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		Model::SelectionModeE getSelectionMode()const { return m_mode; };

		void setHideDifference(int number,bool visible);

	private:
		typedef struct _HideDifference_t
		{
			int  number;
			bool visible;
		} HideDifferenceT;

		typedef std::vector<HideDifferenceT> HideDifferenceList;

		Model::SelectionModeE m_mode;
		HideDifferenceList m_diff;
};

class MU_InvertNormal : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void addTriangle(int triangle);

	private:
		int_list m_triangles;
};

class MU_MoveUnanimated : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void addPosition(const Model::Position &pos, double x, double y, double z,
				double oldx, double oldy, double oldz);

	private:

		struct MovePrimitiveT : Model::Position
		{
			double x;
			double y;
			double z;
			double oldx;
			double oldy;
			double oldz;
		};

		//TODO: Use unsorted_map/Position.
		typedef sorted_list<MovePrimitiveT> MovePrimitiveList;

		MovePrimitiveList m_objects;
};

class MU_SetObjectUnanimated : public ModelUndo
{
public:

	MU_SetObjectUnanimated(const Model::Position &pos):pos(pos)
	{}

	void undo(Model *);
	void redo(Model *);
	bool combine(Undo *);

	unsigned size();

	enum Vec{ Abs=0,Rot,Scale,Rel };

	void setXYZ(Model*, double object[3], const double xyz[3]);

	void _call_setter(Model*,double*);

private:

	Model::Position pos;
	int vec;
	double v[3], vold[3];
};

class MU_SetTexture : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setTexture(unsigned groupNumber, int newTexture, int oldTexture);

	private:
		typedef struct _SetTexture_t
		{
			unsigned groupNumber;
			int newTexture;
			int oldTexture;
		} SetTextureT;
		typedef std::vector<SetTextureT> SetTextureList;

		SetTextureList m_list;
};

class MU_AddVertex : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void redoRelease();

		unsigned size();

		void addVertex(unsigned index,Model::Vertex *vertex);

	private:
		typedef struct _AddVertex_t
		{
			unsigned index;
			Model::Vertex *vertex;
		} AddVertexT;
		typedef std::vector<AddVertexT> AddVertexList;

		AddVertexList m_list;
};

class MU_AddTriangle : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void redoRelease();

		unsigned size();

		void addTriangle(unsigned index,Model::Triangle *);

	private:
		typedef struct _AddTriangle_t
		{
			unsigned index;
			Model::Triangle *triangle;
		} AddTriangleT;
		typedef std::vector<AddTriangleT> AddTriangleList;

		AddTriangleList m_list;
};

class MU_AddGroup : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void redoRelease();

		unsigned size();

		void addGroup(unsigned index,Model::Group *);

	private:
		typedef struct _AddGroup_t
		{
			unsigned index;
			Model::Group *group;
		} AddGroupT;
		typedef std::vector<AddGroupT> AddGroupList;

		AddGroupList m_list;
};

class MU_AddTexture : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void redoRelease();

		unsigned size();

		void addTexture(unsigned index,Model::Material *);

	private:
		typedef struct _AddTexture_t
		{
			unsigned index;
			Model::Material *texture;
		} AddTextureT;
		typedef std::vector<AddTextureT> AddTextureList;

		AddTextureList m_list;
};

class MU_SetTextureCoords : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void addTextureCoords(unsigned triangle, unsigned vertexIndex, float s, float t, float oldS, float oldT);

	private:
		typedef struct _SetTextureCoords_t
		{
			unsigned triangle;
			unsigned vertexIndex;
			float t;
			float s;
			float oldT;
			float oldS;
			bool operator<(const struct _SetTextureCoords_t &rhs)const
			{
				return (this->triangle<rhs.triangle
					  ||(this->triangle==rhs.triangle&&this->vertexIndex<rhs.vertexIndex));
			};
			bool operator==(const struct _SetTextureCoords_t &rhs)const
			{
				return (this->triangle==rhs.triangle&&this->vertexIndex==rhs.vertexIndex);
			};
		} SetTextureCoordsT;

		typedef sorted_list<SetTextureCoordsT> STCList;

		STCList m_list;
};

class MU_AddToGroup : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void addToGroup(unsigned groupNum, unsigned triangleNum);

	private:
		typedef struct _AddToGroup_t
		{
			unsigned groupNum;
			unsigned triangleNum;
		} AddToGroupT;
		typedef std::vector<AddToGroupT> AddToGroupList;

		AddToGroupList m_list;
};

class MU_RemoveFromGroup : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void removeFromGroup(unsigned groupNum, unsigned triangleNum);

	private:
		typedef struct _RemoveFromGroup_t
		{
			unsigned groupNum;
			unsigned triangleNum;
		} RemoveFromGroupT;
		typedef std::vector<RemoveFromGroupT> RemoveFromGroupList;

		RemoveFromGroupList m_list;
};

class MU_DeleteTriangle : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void undoRelease();

		unsigned size();

		void deleteTriangle(unsigned triangleNum,Model::Triangle *triangle);

	private:
		typedef struct _DeleteTriangle_t
		{
			unsigned triangleNum;
			Model::Triangle *triangle;
		} DeleteTriangleT;
		typedef std::vector<DeleteTriangleT> DeleteTriangleList;

		DeleteTriangleList m_list;
};

class MU_DeleteVertex : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void undoRelease();

		unsigned size();

		void deleteVertex(unsigned vertexNum,Model::Vertex *vertex);

	private:
		typedef struct _DeleteVertex_t
		{
			unsigned vertexNum;
			Model::Vertex *vertex;
		} DeleteVertexT;
		typedef std::vector<DeleteVertexT> DeleteVertexList;

		DeleteVertexList m_list;
};

class MU_DeleteBoneJoint : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void undoRelease();

		unsigned size();

		void deleteBoneJoint(unsigned jointNum,Model::Joint *joint);

	private:
		unsigned		 m_jointNum;
		Model::Joint *m_joint;
};

class MU_DeletePoint : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void undoRelease();

		unsigned size();

		void deletePoint(unsigned pointNum,Model::Point *point);

	private:
		unsigned		 m_pointNum;
		Model::Point *m_point;
};

class MU_DeleteProjection : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void undoRelease();

		unsigned size();

		void deleteProjection(unsigned projNum,Model::TextureProjection *proj);

	private:
		unsigned		 m_projNum;
		Model::TextureProjection *m_proj;
};

class MU_AddBoneJoint : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void redoRelease();

		unsigned size();

		void addBoneJoint(unsigned jointNum,Model::Joint *joint);

	private:
		unsigned		 m_jointNum;
		Model::Joint *m_joint;
};

class MU_AddPoint : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void redoRelease();

		unsigned size();

		void addPoint(unsigned pointNum,Model::Point *point);

	private:
		unsigned		 m_pointNum;
		Model::Point *m_point;
};

class MU_AddProjection : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void redoRelease();

		unsigned size();

		void addProjection(unsigned pointNum,Model::TextureProjection *point);

	private:
		unsigned		 m_projNum;
		Model::TextureProjection *m_proj;
};

class MU_DeleteGroup : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void undoRelease();

		unsigned size();

		void deleteGroup(unsigned groupNum,Model::Group *group);

	private:
		typedef struct _DeleteGroup_t
		{
			unsigned groupNum;
			Model::Group *group;
		} DeleteGroupT;
		typedef std::vector<DeleteGroupT> DeleteGroupList;

		DeleteGroupList m_list;
};

class MU_DeleteTexture : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void undoRelease();

		unsigned size();

		void deleteTexture(unsigned textureNum,Model::Material *texture);

	private:
		typedef struct _DeleteTexture_t
		{
			unsigned textureNum;
			Model::Material *texture;
		} DeleteTextureT;
		typedef std::vector<DeleteTextureT> DeleteTextureList;

		DeleteTextureList m_list;
};

class MU_SetLightProperties : public ModelUndo
{
	public:
		enum _LightType_e
		{
			LightAmbient  = 0,
			LightDiffuse,
			LightSpecular,
			LightEmissive,
			LightTypeMax  
		};
		typedef enum _LightType_e LightTypeE;

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setLightProperties(unsigned textureNum,LightTypeE type, const float *newLight, const float *oldLight);

	private:
		typedef struct _LightProperties_t
		{
			unsigned textureNum;
			float newLight[LightTypeMax][4];
			float oldLight[LightTypeMax][4];
			bool  isSet[LightTypeMax];
		} LightPropertiesT;

		typedef std::vector<LightPropertiesT> LightPropertiesList;

		LightPropertiesList m_list;
};

class MU_SetShininess : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setShininess(unsigned textureNum, const float &newValue, const float &oldValue);

	private:
		typedef struct _Shininess_t
		{
			unsigned textureNum;
			float oldValue;
			float newValue;
		} ShininessT;

		typedef std::vector<ShininessT> ShininessList;

		std::vector<ShininessT> m_list;
};

class MU_SetTextureName : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setTextureName(unsigned textureNum, const char *newName, const char *oldName);

	private:

		unsigned m_textureNum;
		std::string	m_newName;
		std::string	m_oldName;
};

class MU_SetTriangleVertices : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setTriangleVertices(unsigned triangleNum,
				unsigned newV1, unsigned newV2, unsigned newV3,
				unsigned oldV1, unsigned oldV2, unsigned oldV3);

	private:
		typedef struct _TriangleVertices_t
		{
			unsigned triangleNum;
			unsigned newVertices[3];
			unsigned oldVertices[3];
		} TriangleVerticesT;

		typedef std::vector<TriangleVerticesT> TriangleVerticesList;

		TriangleVerticesList m_list;
};

class MU_SubdivideSelected : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size(){ return sizeof(MU_SubdivideSelected); };

	private:
};

class MU_SubdivideTriangle : public ModelUndo
{
	public:

		void undo(Model*);
		void redo(Model*){ /*NOP*/ }
		bool combine(Undo *);

		unsigned size();

		void subdivide(unsigned a, unsigned b, unsigned c, unsigned d);
		void addVertex(unsigned v);

	private:

		typedef struct _SubdivideTriangle_t
		{
			unsigned a;
			unsigned b;
			unsigned c;
			unsigned d;
		} SubdivideTriangleT;

		//FIX ME?
		typedef std::vector<SubdivideTriangleT> SubdivideTriangleList;
		SubdivideTriangleList m_list;
		std::vector<unsigned> m_vlist;
};

class MU_ChangeAnimState : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		MU_ChangeAnimState(Model *newInfo, const Model::RestorePoint &old)
		:m_new(newInfo->makeRestorePoint()),m_old(old){}

	protected:

		Model::RestorePoint m_new,m_old;
};

class MU_SetAnimName : public ModelUndo
{
	public:

		void undo(Model*),redo(Model*);

		bool combine(Undo *);

		unsigned size();

		void setName(unsigned animNum, const char *newName, const char *oldName);

	private:

		unsigned m_animNum;
		std::string	m_newName;
		std::string	m_oldName;
};

class MU_SetAnimFrameCount : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		void undoRelease();
		void redoRelease();

		unsigned size();

		void setAnimFrameCount(unsigned animNum, unsigned newCount, unsigned oldCount, unsigned where);

		void removeTimeTable(const std::vector<double> &tt)
		{
			if(m_oldCount>m_newCount) //Reminder: Dinkumware range checks +/- operators
			m_timetable.assign(tt.begin()+m_where,tt.begin()+(m_where+m_oldCount-m_newCount)); 
		}

		Model::FrameAnimData *removeVertexData(){ return &m_vertices; }

	private:

		unsigned m_animNum;
		unsigned m_newCount;
		unsigned m_oldCount;		
		unsigned m_where; 

		std::vector<double> m_timetable;
		Model::FrameAnimData m_vertices;
};

class MU_SetAnimFPS : public ModelUndo
{
	public:

		void undo(Model*),redo(Model*);

		bool combine(Undo *);

		unsigned size();

		void setFPS(unsigned animNum, double newFps, double oldFps);

	private:

		unsigned m_animNum;
		double	m_newFPS;
		double	m_oldFPS;
};

class MU_SetAnimWrap : public ModelUndo
{
	public:
		
		void undo(Model*),redo(Model*);

		bool combine(Undo *);

		unsigned size(){ return sizeof(MU_SetAnimWrap); }

		void setAnimWrap(unsigned animNum,bool newLoop,bool oldLoop);

	private:

		unsigned m_animNum;
		bool	m_newLoop;
		bool	m_oldLoop;
};

//https://github.com/zturtleman/mm3d/issues/106
class MU_SetAnimTime : public ModelUndo
{
	public:

		void undo(Model*),redo(Model*);

		bool combine(Undo *);

		unsigned size(){ return sizeof(MU_SetAnimTime); }

		void setAnimFrameTime(unsigned animNum, unsigned frame, double newTime, double oldTime);
		void setAnimTimeFrame(unsigned animNum, double newTime, double oldTime)
		{
			setAnimFrameTime(animNum,INT_MAX,newTime,oldTime);
		}

	private:

		unsigned m_animNum;
		unsigned m_animFrame;
		double	m_newTime;
		double	m_oldTime;
};

class MU_SetObjectKeyframe : public ModelUndo
{
	protected:

		void undo(Model*),redo(Model*);

		bool combine(Undo *);

	public:

		unsigned size();

		MU_SetObjectKeyframe(unsigned anim, unsigned frame, Model::KeyType2020E isRotation)
		:m_anim(anim),m_frame(frame),m_isRotation(isRotation)
		{}

		void addKeyframe(Model::Position j, bool isNew,
			double x, double y, double z, Model::Interpolate2020E e,
				double oldx, double oldy, double oldz, Model::Interpolate2020E olde);

	private:

		struct SetKeyFrameT : Model::Position
		{
			using Position::operator=; //C++

			bool isNew;
			double x;
			double y;
			double z;
			double oldx;
			double oldy;
			double oldz;
			Model::Interpolate2020E e,olde;
		};
		typedef sorted_list<SetKeyFrameT> SetKeyframeList;

		SetKeyframeList m_keyframes;
		unsigned m_anim;
		unsigned m_frame;
		Model::KeyType2020E m_isRotation;
};

class MU_DeleteObjectKeyframe : public ModelUndo
{
	protected:

		void undo(Model*),redo(Model*);

		bool combine(Undo *);

	public:

		void undoRelease();

		unsigned size();

		MU_DeleteObjectKeyframe(unsigned anim, unsigned frame)
		:m_anim(anim),m_frame(frame){}

		void deleteKeyframe(Model::Keyframe *keyframe);

	private:
		typedef std::vector<Model::Keyframe*> DeleteKeyframeList;

		DeleteKeyframeList m_list;
		unsigned m_anim,m_frame;
};

class MU_SetObjectName : public ModelUndo
{
	public:

		MU_SetObjectName(const Model::Position &obj):m_obj(obj)
		{}

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setName(const char *newName, const char *oldName);

	private:

		Model::Position m_obj;
		std::string	m_newName;
		std::string	m_oldName;
};

class MU_SetProjectionType : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setType(unsigned projection, int newType, int oldType);

	private:

		unsigned m_projection;
		int	m_newType;
		int	m_oldType;
};

class MU_MoveFrameVertex : public ModelUndo
{
	public:

		void undo(Model*),redo(Model*);

		bool combine(Undo*);

		unsigned size();

		MU_MoveFrameVertex(unsigned anim, unsigned frame)
		:m_anim(anim),m_frame(frame){}

		void addVertex(int v, double x, double y, double z,
		Model::Interpolate2020E e, Model::FrameAnimVertex *old, bool sort=true);

	private:

		struct MoveFrameVertexT
		{
			int number;
			double x;
			double y;
			double z;
			double oldx;
			double oldy;
			double oldz;
			Model::Interpolate2020E e,olde;

			bool operator<(const MoveFrameVertexT &rhs)const
			{
				return number<rhs.number;
			};
			bool operator==(const MoveFrameVertexT &rhs)const
			{
				return number==rhs.number;
			};
		};
		typedef sorted_list<MoveFrameVertexT> MoveFrameVertexList;

		friend Model; //2020
		MoveFrameVertexList m_vertices;
		unsigned m_anim;
		unsigned m_frame;
		
		void addVertex(MoveFrameVertexT&); //2020
};

class MU_SetPositionInfluence : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setPositionInfluence(bool isAdd, const Model::Position &pos, unsigned index, const Model::InfluenceT &influence);

	private:
		bool m_isAdd;
		Model::Position m_pos;
		unsigned m_index;
		Model::InfluenceT m_influence;
};

class MU_UpdatePositionInfluence : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void updatePositionInfluence(const Model::Position &pos,
				const Model::InfluenceT &newInf,
				const Model::InfluenceT &oldInf);

	private:
		Model::Position m_pos;
		Model::InfluenceT m_newInf;
		Model::InfluenceT m_oldInf;
};

/*UNUSED
class MU_SetVertexBoneJoint : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setVertexBoneJoint(const unsigned &vertex,
				const int &newBone, const int &oldBone);

	private:
		typedef struct _SetJoint_t
		{
			unsigned vertex;
			int newBone;
			int oldBone;
		} SetJointT;
		typedef std::vector<SetJointT> SetJointList;

		SetJointList m_list;
};*/

/*UNUSED
class MU_SetPointBoneJoint : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setPointBoneJoint(const unsigned &point,
				const int &newBone, const int &oldBone);

	private:
		typedef struct _SetJoint_t
		{
			unsigned point;
			int newBone;
			int oldBone;
		} SetJointT;
		typedef std::vector<SetJointT> SetJointList;

		SetJointList m_list;
};*/

class MU_SetTriangleProjection : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setTriangleProjection(const unsigned &vertex,
				const int &newProj, const int &oldProj);

	private:
		typedef struct _SetProjection_t
		{
			unsigned triangle;
			int newProj;
			int oldProj;
		} SetProjectionT;
		typedef std::vector<SetProjectionT> SetProjectionList;

		SetProjectionList m_list;
};

class MU_AddAnimation : public ModelUndo
{
	public:

		MU_AddAnimation(unsigned a, Model::Animation *p)
		:m_anim(a),m_animp(p){}

		void undo(Model*),redo(Model*);

		bool combine(Undo *);

		void redoRelease();

		unsigned size();

	private:
		
		unsigned m_anim;
		Model::Animation *m_animp;
};

class MU_DeleteAnimation : public ModelUndo
{
	public:

		MU_DeleteAnimation(unsigned a, Model::Animation *p, bool dataOnly=false)
		:m_anim(a),m_animp(p),m_dataOnly(dataOnly){}

		void undo(Model*),redo(Model*);

		bool combine(Undo *);

		void undoRelease();

		unsigned size();

		Model::FrameAnimData *removeVertexData(){ return &m_vertices; }

	private:
		
		unsigned m_anim;
		Model::Animation *m_animp;
		bool m_dataOnly;

		Model::FrameAnimData m_vertices;
};

class MU_SetJointParent : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *){ return false; };

		unsigned size(){ return sizeof(MU_SetJointParent); };

		MU_SetJointParent(unsigned joint, int newParent, Matrix &redo, Model::Joint *old)
		:m_joint(joint),m_new(newParent)
		,m_old(old->m_parent),m_redo(redo),m_undo(old->m_relative)
		{}

	protected:
		unsigned m_joint;
		int m_new,m_old;
		Matrix m_undo,m_redo; //2021
};

class MU_SetProjectionRange : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *){ return false; }; //IMPLEMENT ME

		unsigned size(){ return sizeof(MU_SetProjectionRange); };

		void setProjectionRange(const unsigned &proj,
				double newXMin, double newYMin, double newXMax, double newYMax,
				double oldXMin, double oldYMin, double oldXMax, double oldYMax);

	protected:
		unsigned m_proj;
		double	m_newRange[4];
		double	m_oldRange[4];
};

class MU_SetGroupSmooth : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setGroupSmooth(const unsigned &group,
				const uint8_t &newSmooth, const uint8_t &oldSmooth);

	private:
		typedef struct _SetSmooth_t
		{
			unsigned group;
			uint8_t newSmooth;
			uint8_t oldSmooth;
		} SetSmoothT;
		typedef std::vector<SetSmoothT> SetSmoothList;

		SetSmoothList m_list;
};

class MU_SetGroupAngle : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setGroupAngle(const unsigned &group,
				const uint8_t &newAngle, const uint8_t &oldAngle);

	private:
		typedef struct _SetAngle_t
		{
			unsigned group;
			uint8_t newAngle;
			uint8_t oldAngle;
		} SetAngleT;
		typedef std::vector<SetAngleT> SetAngleList;

		SetAngleList m_list;
};

class MU_SetGroupName : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setGroupName(unsigned groupNum, const char *newName, const char *oldName);

	private:

		unsigned m_groupNum;
		std::string m_newName;
		std::string m_oldName;
};

class MU_MoveAnimation : public ModelUndo
{
	public:

		MU_MoveAnimation(unsigned oldIndex, unsigned newIndex, int typeDiff=0)
		:m_oldIndex(oldIndex),m_newIndex(newIndex),m_typeDiff(typeDiff)
		{}

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

	private:

		unsigned m_oldIndex,m_newIndex; int m_typeDiff;
};

/*
class MU_SetFrameAnimVertexCount : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *){ return false; };

		unsigned size();

		void setCount(const unsigned &newCount, const unsigned &oldCount);

	private:

		unsigned m_newCount;
		unsigned m_oldCount;
};

class MU_SetFrameAnimPointCount : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *){ return false; };

		unsigned size();

		void setCount(const unsigned &newCount, const unsigned &oldCount);

	private:

		unsigned m_newCount;
		unsigned m_oldCount;
};*/

class MU_SetMaterialClamp : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setMaterialClamp(const unsigned &material, const bool &isS,
				const bool &newClamp, const bool &oldClamp);

	private:
		unsigned m_material;
		bool m_isS;
		bool m_oldClamp;
		bool m_newClamp;
};

class MU_SetMaterialTexture : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void setMaterialTexture(const unsigned &material,
				Texture *newTexture,Texture *oldTexture);

	private:
		unsigned m_material;
		Texture *m_oldTexture;
		Texture *m_newTexture;
};

class MU_AddMetaData : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void addMetaData(const std::string &key,
				const std::string &value);

	private:
		std::string m_key;
		std::string m_value;
};

class MU_UpdateMetaData : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void updateMetaData(const std::string &key,
				const std::string &newValue, const std::string &oldValue);

	private:
		std::string m_key;
		std::string m_newValue;
		std::string m_oldValue;
};

class MU_ClearMetaData : public ModelUndo
{
	public:

		void undo(Model *);
		void redo(Model *);
		bool combine(Undo *);

		unsigned size();

		void clearMetaData(const Model::MetaDataList &list);

	private:
		Model::MetaDataList m_list;
};

//NOTE: This became too complicated for keyframes so 
//it's just an optimization for vertex animation data
class MU_InterpolateSelected : public ModelUndo
{
public:

	MU_InterpolateSelected
	(Model::Interpolate2020E e, unsigned anim, unsigned frame)
	:m_e(e),m_anim(anim),m_frame(frame){}

	void undo(Model *m){ _do(m,true); }
	void redo(Model *m){ _do(m,false); }
	bool combine(Undo*){ return false; }

	unsigned size();

	void addVertex(Model::Interpolate2020E e)
	{
		m_eold.push_back(e);
	}

private:

	Model::Interpolate2020E m_e;
	unsigned m_anim;
	unsigned m_frame;
	std::vector<Model::Interpolate2020E> m_eold;

	//NOTE: Had synchronized animation (AnimRP needed?)
	void _do(Model*,bool);
};

class MU_RemapTrianglesIndices : public ModelUndo
{
public:

	void undo(Model*);
	void redo(Model*);
	bool combine(Undo*){ return false; }

	unsigned size();

	int_list map;

	void swap();
};

#endif //  __MODELUNDO_H
