/*
 * grid_function_util.h
 *
 *  Created on: 17.08.2010
 *      Author: andreasvogel
 */

#ifndef __H__LIBDISCRETIZATION__FUNCTION_SPACE__GRID_FUNCTION_UTIL__
#define __H__LIBDISCRETIZATION__FUNCTION_SPACE__GRID_FUNCTION_UTIL__

#include "./grid_function.h"
#include "lib_algebra/lib_algebra.h"
#include "lib_algebra/operator/debug_writer.h"
#include "lib_discretization/io/vtkoutput.h"
#include <vector>
#include <string>

#ifdef UG_PARALLEL
	#include "pcl/pcl.h"
#endif

namespace ug{


template <typename TFunction>
void ExtractPositions(const TFunction &u, std::vector<MathVector<TFunction::domain_type::dim> > &positions)
{
	typedef typename TFunction::domain_type domain_type;
	const typename domain_type::position_accessor_type& aaPos = u.get_approximation_space().get_domain().get_position_accessor();

	int nr = u.num_dofs();
	positions.resize(nr);

	for(int si = 0; si < u.num_subsets(); ++si)
	{
		for(geometry_traits<VertexBase>::const_iterator iter = u.template begin<VertexBase>(si);
				iter != u.template end<VertexBase>(si); ++iter)
		{
			VertexBase* v = *iter;

			typename TFunction::algebra_index_vector_type ind;

			u.get_inner_algebra_indices(v, ind);

			for(size_t i = 0; i < ind.size(); ++i)
			{
				const size_t index = ind[i];
				positions[index] = aaPos[v];
			}
		}
	}
}

template <class TFunction>
void WriteMatrixToConnectionViewer(const char *filename, const typename TFunction::algebra_type::matrix_type &A, const TFunction &u)
{
	const static int dim = TFunction::domain_type::dim;
	std::vector<MathVector<dim> > positions;

	// get positions of vertices
	ExtractPositions(u, positions);

	// write matrix
	WriteMatrixToConnectionViewer(filename, A, &positions[0], dim);
}

template <class TFunction>
void WriteVectorToConnectionViewer(const char *filename, const typename TFunction::algebra_type::vector_type &b, const TFunction &u)
{
	const static int dim = TFunction::domain_type::dim;
	std::vector<MathVector<dim> > positions;

	// get positions of vertices
	ExtractPositions(u, positions);

	// write matrix
	WriteVectorToConnectionViewer(filename, b, &positions[0], dim);
}

template <typename TGridFunction>
class GridFunctionDebugWriter
	: public IDebugWriter<typename TGridFunction::algebra_type>
{
	public:
	///	type of matrix
		typedef typename TGridFunction::algebra_type algebra_type;

	///	type of vector
		typedef typename algebra_type::vector_type vector_type;

	///	type of matrix
		typedef typename algebra_type::matrix_type matrix_type;

	public:
	///	Constructor
		GridFunctionDebugWriter() :
			m_pGridFunc(NULL),
			bConnViewerOut(true), bVTKOut(true)
		{}

	///	sets the function
		void set_reference_grid_function(const TGridFunction& u)
		{
			m_pGridFunc = &u;
		}

	//	sets if writing to vtk
		void set_vtk_output(bool b) {bVTKOut = b;}

	//	sets if writing to conn viewer
		void set_conn_viewer_output(bool b) {bConnViewerOut = b;}

	///	write vector
		virtual bool write_vector(const vector_type& vec,
		                          const char* filename)
		{
			bool bRet = true;

		//	write to conn viewer
			if(bConnViewerOut)
				bRet &= write_vector_to_conn_viewer(vec, filename);

		//	write to vtk
			if(bVTKOut)
				bRet &= write_vector_to_vtk(vec, filename);

		//	we're done
			return bRet;
		}

	///	write matrix
		virtual bool write_matrix(const matrix_type& mat,
		                          const char* filename)
		{
		//	something to do ?
			if(!bConnViewerOut) return true;

		//	check
			if(m_pGridFunc == NULL)
			{
				UG_LOG("ERROR in 'GridFunctionDebugWriter::write_vector':"
						" No reference grid function set.\n");
				return false;
			}

		//	get fresh name
			std::string name(filename);

		//	search for ending and remove
			size_t found = name.find_first_of(".");
			if(found!=string::npos)	name.resize(found);

		#ifdef UG_PARALLEL
		//	add process number
			int rank = pcl::GetProcRank();
			char ext[20];
			sprintf(ext, "_p%04d", rank);
			name.append(ext);
		#endif

		//	add ending
			name.append(".mat");

		//	write to file
			WriteMatrixToConnectionViewer(name.c_str(), mat, *m_pGridFunc);

		//  we're done
			return true;
		}

	protected:
	///	write vector
		virtual bool write_vector_to_conn_viewer(const vector_type& vec,
		                                         const char* filename)
		{
		//	check
			if(m_pGridFunc == NULL)
			{
				UG_LOG("ERROR in 'GridFunctionDebugWriter::write_vector':"
						" No reference grid function set.\n");
				return false;
			}

		//	get fresh name
			std::string name(filename);

		//	search for ending and remove
			size_t found = name.find_first_of(".");
			if(found!=string::npos)	name.resize(found);

		#ifdef UG_PARALLEL
		//	add process number
			int rank = pcl::GetProcRank();
			char ext[20];
			sprintf(ext, "_p%04d", rank);
			name.append(ext);
		#endif

		//	add ending
		//	\todo: Introduce a ending *.vec for Connection Viewer
			name.append(".mat");

		//	write to file
			WriteVectorToConnectionViewer(name.c_str(), vec, *m_pGridFunc);

		//	we're done
			return true;
		}

		bool write_vector_to_vtk(const vector_type& vec, const char* filename)
		{
		//	check
			if(m_pGridFunc == NULL)
			{
				UG_LOG("ERROR in 'GridFunctionDebugWriter::write_vector':"
						" No reference grid function set.\n");
				return false;
			}

		//	assign all patterns and sizes
			m_vtkFunc.clone_pattern((*m_pGridFunc));
			m_vtkFunc = (*m_pGridFunc);

		//	overwrite values with vector
			m_vtkFunc.assign(vec);

		//	create vtk output
			VTKOutput<TGridFunction> out;

		//	write
			return out.print(filename, m_vtkFunc);
		}

	protected:
	// 	grid function used as reference
		const TGridFunction* m_pGridFunc;

	// 	dummy vector for vtk output
		TGridFunction m_vtkFunc;

	//	flag if write to conn viewer
		bool bConnViewerOut;

	//	flag if write to vtk
		bool bVTKOut;
};

} // end namespace ug

#endif /* __H__LIBDISCRETIZATION__FUNCTION_SPACE__GRID_FUNCTION_UTIL__ */
