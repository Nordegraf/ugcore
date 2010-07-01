//	created by Sebastian Reiter
//	s.b.reiter@googlemail.com
//	y10 m04 d14

#ifndef __H__LIB_GRID__GLOBAL_MULTI_GRID_REFINER__
#define __H__LIB_GRID__GLOBAL_MULTI_GRID_REFINER__

#include <vector>
#include "lib_grid/lg_base.h"
#include "lib_grid/multi_grid.h"

namespace ug
{

class GlobalMultiGridRefiner : public GridObserver
{
	public:
		GlobalMultiGridRefiner();
		GlobalMultiGridRefiner(MultiGrid& mg);
		virtual ~GlobalMultiGridRefiner();

		virtual void grid_to_be_destroyed(Grid* grid);
		
		void assign_grid(MultiGrid& mg);
		void assign_grid(MultiGrid* mg);

	////////////////////////////////
	//	refine
	///	performs refinement on the marked elements.
		void refine();

	protected:
	///	a callback that allows to deny refinement of special vertices
		virtual bool refinement_is_allowed(VertexBase* elem)	{return true;}
	///	a callback that allows to deny refinement of special edges
		virtual bool refinement_is_allowed(EdgeBase* elem)		{return true;}
	///	a callback that allows to deny refinement of special faces
		virtual bool refinement_is_allowed(Face* elem)			{return true;}
	///	a callback that allows to deny refinement of special volumes
		virtual bool refinement_is_allowed(Volume* elem)		{return true;}
		
	///	this method helps derived classes to perform operations directly before actual element refinment is performed.
	/**	Called from the refine() method in each refinement-iteration after
	 *	collect_objects_for_refine().
	 *	Default implementation is empty.*/
		virtual void refinement_step_begins()	{};

	///	this method helps derived classes to perform operations directly after actual element refinment took place.
	/**	Called from the refine() method in each refinement-iteration after
	 *	all scheduled elements had been refined.
	 *	The refine process will either terminate after this method or will
	 *	start a new iteration, if new elements had been marked during refine.
	 *	Default implementation is empty.*/
		virtual void refinement_step_ends()		{};
		
	protected:
		MultiGrid*	m_pMG;
};

}//	end of namespace

#endif
