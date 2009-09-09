//===========================================================================
//
// File: GridInterfaceEuler.hpp
//
// Created: Mon Jun 15 12:53:38 2009
//
// Author(s): Atgeirr F Rasmussen <atgeirr@sintef.no>
//            B�rd Skaflestad     <bard.skaflestad@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
  Copyright 2009 SINTEF ICT, Applied Mathematics.
  Copyright 2009 Statoil ASA.

  This file is part of The Open Reservoir Simulator Project (OpenRS).

  OpenRS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OpenRS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OpenRS.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPENRS_GRIDINTERFACEEULER_HEADER
#define OPENRS_GRIDINTERFACEEULER_HEADER

#include "config.h"
#include <climits>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/scoped_ptr.hpp>

#include <dune/common/fvector.hh>
#include <dune/grid/common/referenceelements.hh>
#include <dune/grid/common/mcmgmapper.hh>

namespace Dune
{


    namespace GIE
    {
	template <class GridInterface, class EntityPointerType>
	class Cell;

	/// @brief
	/// @todo Doc me!
	/// @tparam
	/// @param
	template <class GridInterface>
	class Intersection : public boost::iterator_facade<Intersection<GridInterface>,
							   const Intersection<GridInterface>,
							   boost::forward_traversal_tag>
	{
	public:

	    /// @brief
	    /// @todo Doc me!
	    typedef typename GridInterface::DuneIntersectionIterator DuneIntersectionIter;
	    /// @brief
	    /// @todo Doc me!
	    /// @param
	    Intersection()
		: pgrid_(0), iter_(), local_index_(-1)
	    {
	    }
	    /// @brief
	    /// @todo Doc me!
	    /// @param
	    Intersection(const GridInterface& grid,
                         const DuneIntersectionIter& it,
			 const int local_index)
		: pgrid_(&grid), iter_(it), local_index_(local_index)
	    {
	    }
	    /// @brief
	    /// @todo Doc me!
	    typedef typename GridInterface::GridType::ctype Scalar;
	    typedef FieldVector<Scalar, GridInterface::GridType::dimension> Vector;
	    typedef FieldVector<Scalar, GridInterface::GridType::dimension - 1> LocalVector;
	    typedef int Index;
	    typedef GIE::Cell<GridInterface, typename GridInterface::GridType::template Codim<0>::EntityPointer> Cell;
	    /// @brief
	    /// @todo Doc me!
	    enum { BoundaryMarkerIndex = -1, LocalEndIndex = INT_MAX };

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Scalar area() const
	    {
		return iter_->geometry().volume();
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Vector centroid() const
	    {
		return iter_->geometry().global(localCentroid());
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Vector normal() const
	    {
		return iter_->unitOuterNormal(localCentroid());
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    bool boundary() const
	    {
		return iter_->boundary();
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    int boundaryId() const
	    {
		return iter_->boundaryId();
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Cell cell() const
	    {
		return Cell(*pgrid_, iter_->inside());
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Index cellIndex() const
	    {
		return pgrid_->mapper().map(*iter_->inside());
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Cell neighbourCell() const
	    {
		return Cell(*pgrid_, iter_->outside());
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Index neighbourCellIndex() const
	    {
		if (iter_->boundary()) {
		    return BoundaryMarkerIndex;
		} else {
		    return pgrid_->mapper().map(*iter_->outside());
		}
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Index index() const
	    {
		return pgrid_->faceIndex(cellIndex(), localIndex());
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Index localIndex() const
	    {
		return local_index_;
	    }

	    /// @brief
	    /// @todo Doc me!
	    /// @return
	    Scalar neighbourCellVolume() const
	    {
		return iter_->outside()->geometry().volume();
	    }

	    /// Used by iterator facade.
	    const Intersection& dereference() const
	    {
		return *this;
	    }
	    /// Used by iterator facade.
	    bool equal(const Intersection& other) const
	    {
		// Note that we do not compare the local_index_ members,
		// since they may or may not be equal for end iterators.
		return iter_ == other.iter_;
	    }
	    /// Used by iterator facade.
	    void increment()
	    {
		++iter_;
		++local_index_;
	    }
	    /// Gives an ordering of intersections.
	    bool operator<(const Intersection& other) const
	    {
		if (cellIndex() == other.cellIndex()) {
		    return localIndex() < other.localIndex();
		} else {
		    return cellIndex() < other.cellIndex();
		}
	    }
	private:
	    const GridInterface* pgrid_;
	    DuneIntersectionIter iter_;
	    int local_index_;

	    LocalVector localCentroid() const
	    {
		typedef Dune::ReferenceElements<Scalar, GridInterface::GridType::dimension-1> RefElems;
		return RefElems::general(iter_->type()).position(0,0);
	    }

	};


	template <class GridInterface, class EntityPointerType>
	class Cell
	{
	public:
	    Cell()
		: pgrid_(0), iter_()
	    {
	    }
	    Cell(const GridInterface& grid,
		 const EntityPointerType& it)
		: pgrid_(&grid), iter_(it)
	    {
	    }
	    typedef GIE::Intersection<GridInterface> FaceIterator;
	    typedef typename FaceIterator::Vector Vector;
	    typedef typename FaceIterator::Scalar Scalar;
	    typedef typename FaceIterator::Index Index;

	    FaceIterator facebegin() const
	    {
		return FaceIterator(*pgrid_, iter_->ileafbegin(), 0);
	    }

	    FaceIterator faceend() const
	    {
		return FaceIterator(*pgrid_, iter_->ileafend(),
                                    FaceIterator::LocalEndIndex);
	    }

	    Scalar volume() const
	    {
		return iter_->geometry().volume();
	    }

	    Vector centroid() const
	    {
		typedef Dune::ReferenceElements<Scalar, GridInterface::GridType::dimension> RefElems;
		Vector localpt
		    = RefElems::general(iter_->type()).position(0,0);
		return iter_->geometry().global(localpt);
	    }

	    Index index() const
	    {
                return pgrid_->mapper().map(*iter_);
	    }
	protected:
	    const GridInterface* pgrid_;
	    EntityPointerType iter_;
	};


	template <class GridInterface>
	class CellIterator
	    : public boost::iterator_facade<CellIterator<GridInterface>,
					    const CellIterator<GridInterface>,
					    boost::forward_traversal_tag>,
	      public Cell<GridInterface, typename GridInterface::GridType::template Codim<0>::LeafIterator>
	{
	private:
	    typedef typename GridInterface::GridType::template Codim<0>::LeafIterator DuneCellIter;
	    typedef Cell<GridInterface, DuneCellIter> CellType;
	public:
	    typedef typename CellType::Vector Vector;
	    typedef typename CellType::Scalar Scalar;
	    typedef typename CellType::Index Index;

	    CellIterator(const GridInterface& grid, DuneCellIter it)
		: CellType(grid, it)
	    {
	    }
	    /// Used by iterator facade.
	    const CellIterator& dereference() const
	    {
		return *this;
	    }
	    /// Used by iterator facade.
	    bool equal(const CellIterator& other) const
	    {
		return CellType::iter_ == other.CellType::iter_;
	    }
	    /// Used by iterator facade.
	    void increment()
	    {
		++CellType::iter_;
	    }
	};

        template<int dim>
        struct AllCellsLayout {
            bool contains(GeometryType gt) { return gt.dim() == dim; }
        };

    } // namespace GIE


    template <class DuneGrid>
    class GridInterfaceEuler
    {
    public:
        typedef LeafMultipleCodimMultipleGeomTypeMapper<DuneGrid, GIE::AllCellsLayout> Mapper;
	typedef typename DuneGrid::LeafIntersectionIterator DuneIntersectionIterator;
	typedef DuneGrid GridType;
	typedef GridInterfaceEuler<DuneGrid> InterfaceType;
	typedef GIE::CellIterator<InterfaceType> CellIterator;
	typedef typename CellIterator::Vector Vector;
	typedef typename CellIterator::Scalar Scalar;
	typedef typename CellIterator::Index Index;
	enum { Dimension = DuneGrid::dimension };

	GridInterfaceEuler()
	    : pgrid_(0)
	{
	}
	explicit GridInterfaceEuler(const DuneGrid& grid)
	    : pgrid_(&grid), pmapper_(new Mapper(grid))
	{
	}
	void init(const DuneGrid& grid)
	{
	    pgrid_ = &grid;
            pmapper_.reset(new Mapper(grid));
	}
	CellIterator cellbegin() const
	{
	    return CellIterator(*this, grid().template leafbegin<0>());
	}
	CellIterator cellend() const
	{
	    return CellIterator(*this, grid().template leafend<0>());
	}
        int numberOfCells() const
        {
            return grid().size(0);
        }
	const DuneGrid& grid() const
	{
	    ASSERT(pgrid_);
	    return *pgrid_;
	}
	const Mapper& mapper() const
	{
	    ASSERT (pmapper_);
	    return *pmapper_;
	}
    private:
	const DuneGrid* pgrid_;
        boost::scoped_ptr<Mapper> pmapper_;
    };



} // namespace Dune


#endif // OPENRS_GRIDINTERFACEEULER_HEADER
