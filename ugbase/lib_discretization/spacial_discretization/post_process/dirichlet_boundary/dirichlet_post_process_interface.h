/*
 * dirichlet_post_process_interface.h
 *
 *  Created on: 04.08.2010
 *      Author: andreasvogel
 */

#ifndef __H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__POST_PROCESS__DIRICHLET_BOUNDARY__DIRICHLET_POST_PROCESS_INTERFACE__
#define __H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__POST_PROCESS__DIRICHLET_BOUNDARY__DIRICHLET_POST_PROCESS_INTERFACE__

// extern headers
#include <vector>

// intern headers
#include "lib_discretization/common/local_algebra.h"

namespace ug{

enum IDirichletPostProcessNeed {
	IDPPN_NONE = 0,
	IDPPN_DEFECT = 1 << 0,
	IDPPN_JACOBIAN = 1 << 1,
	IDPPN_LINEAR = 1 << 2,
	IDPPN_SOLUTION = 1 << 3
};

template <typename TAlgebra>
class IDirichletPostProcess{
	protected:
		// algebra type
		typedef TAlgebra algebra_type;

		// local matrix type
		typedef typename algebra_type::matrix_type matrix_type;

		// local vector type
		typedef typename algebra_type::vector_type vector_type;

		// local index type
		typedef LocalIndices local_index_type;

	public:
		// sets the geometric object type
		// ATTENTION: type must be set, before other public functions can be called
		bool set_geometric_object_type(int id, IDirichletPostProcessNeed need);

		// preparing and finishing of loop
		bool prepare_element_loop()						{return (this->*(m_vPrepareElementLoopFunc[m_id]))();}
		bool prepare_element(GeometricObject* obj)		{return (this->*(m_vPrepareElementFunc[m_id]))(obj);}
		bool finish_element_loop()						{return (this->*(m_vFinishElementLoopFunc[m_id]))();}

		// post processing  of Jacobian
		bool post_process_J(matrix_type& J, const local_index_type& ind, number time=0.0) 	{return (this->*(m_vPostProcessJFunc[m_id]))(J, ind, time);}

		// post processing  of defect
		bool post_process_d(vector_type& d, const local_index_type& ind, number time=0.0)	{return (this->*(m_vPostProcessDFunc[m_id]))(d, ind, time);}

		// post processing of right hand side for linear case
		bool post_process_f(vector_type& d, const local_index_type& ind, number time=0.0) 	{return (this->*(m_vPostProcessFFunc[m_id]))(d, ind, time);}

		// set solution
		bool set_solution(vector_type& x, const local_index_type& ind, number time=0.0) 	{return (this->*(m_vSetSolutionFunc[m_id]))(x, ind, time);}

		// virtual destructor
		virtual ~IDirichletPostProcess() {}

	protected:
		// register the functions
		template <typename TAssFunc> void register_prepare_element_loop_function(int id, TAssFunc func);
		template <typename TAssFunc> void register_prepare_element_function(int id, TAssFunc func);
		template <typename TAssFunc> void register_finish_element_loop_function(int id, TAssFunc func);

		template <typename TAssFunc> void register_post_process_J_function(int id, TAssFunc func);
		template <typename TAssFunc> void register_post_process_d_function(int id, TAssFunc func);
		template <typename TAssFunc> void register_post_process_f_function(int id, TAssFunc func);
		template <typename TAssFunc> void register_set_solution_function(int id, TAssFunc func);

	protected:
		// checks if the needed functions are registered for the id type
		bool function_registered(int id, IDirichletPostProcessNeed need);

		// checks if the functions are present
		bool prepare_element_loop_function_registered(int id);
		bool prepare_element_function_registered(int id);
		bool finish_element_loop_function_registered(int id);

		// checks if the functions are present
		bool post_process_J_function_registered(int id);
		bool post_process_d_function_registered(int id);
		bool post_process_f_function_registered(int id);
		bool set_solution_function_registered(int id);

	private:
		// types of loop function pointers
		typedef bool (IDirichletPostProcess<TAlgebra>::*PrepareElementLoopFunc)();
		typedef bool (IDirichletPostProcess<TAlgebra>::*PrepareElementFunc)(GeometricObject* obj);
		typedef bool (IDirichletPostProcess<TAlgebra>::*FinishElementLoopFunc)();

		// types of Jacobian assemble functions
		typedef bool (IDirichletPostProcess<TAlgebra>::*PostProcessJFunc)(matrix_type& J, const local_index_type& ind, number time);

		// types of Defect assemble functions
		typedef bool (IDirichletPostProcess<TAlgebra>::*PostProcessDFunc)(vector_type& d, const local_index_type& ind, number time);

		// types of right hand side assemble functions
		typedef bool (IDirichletPostProcess<TAlgebra>::*PostProcessFFunc)(vector_type& d, const local_index_type& ind, number time);

		// types of set solution functions
		typedef bool (IDirichletPostProcess<TAlgebra>::*SetSolutionFunc)(vector_type& x, const local_index_type& ind, number time);

	private:
		// loop function pointers
		std::vector<PrepareElementLoopFunc> m_vPrepareElementLoopFunc;
		std::vector<PrepareElementFunc> 	m_vPrepareElementFunc;
		std::vector<FinishElementLoopFunc> 	m_vFinishElementLoopFunc;

		// Jacobian function pointers
		std::vector<PostProcessJFunc> 	m_vPostProcessJFunc;

		// Defect function pointers
		std::vector<PostProcessDFunc> 	m_vPostProcessDFunc;

		// Rhs function pointers
		std::vector<PostProcessFFunc> 	m_vPostProcessFFunc;

		// Rhs function pointers
		std::vector<SetSolutionFunc> 	m_vSetSolutionFunc;
	protected:
		// current Geometric Object
		int m_id;

};

} // end namespace ug

#include "dirichlet_post_process_interface_impl.h"

#endif /* __H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__POST_PROCESS__DIRICHLET_BOUNDARY__DIRICHLET_POST_PROCESS_INTERFACE__ */
