--------------------------------------------------------------------------------
--	tut08_nonlinear_conv_diff_using_self_coupling.lua
--
--	This tutorial is used to show tha ability to compute non-linear problems.
--	The problem itself will be a simple convection-diffusion equation that is
--	non-linear since the coefficients are non-linear.
--------------------------------------------------------------------------------

-- Since ug supports a bunch of different algebra modules we will choose one here.
-- This should always be the first thing you do in an ug-script.
-- The cpu-algebra is fine for now.
InitAlgebra(CPUAlgebraChooser())

-- We will include a script file which defines some methods often used.
-- Loaded methods are all found in the library util.
ug_load_script("../ug_util.lua")

-- To keep the script flexible we will now define some variables which have
-- a default value that can be overwritten by command line arguments.

-- Depending on the dimension we will choose our domain object
-- (either 1d, 2d or 3d) and associated discretization objects. Note that
-- we're using some methods defined in "ug_util.lua" here.
dim = util.GetParamNumber("-dim", 1) -- default dimension is 1.

-- We also need a filename for the grid that shall be loaded.
if 		dim == 1 then gridName = util.GetParam("-grid", "unit_line_2.ugx")
elseif 	dim == 2 then gridName = util.GetParam("-grid", "unit_square_quads_8x8.ugx")
elseif 	dim == 3 then gridName = util.GetParam("-grid", "unit_cube_hex.ugx")
else print("Choosen Dimension not supported. Exiting."); exit(); end

-- Since we want to save the domains grid to a file, we also need an output file.
-- Note that we only use a prefix here, since we want to attach the process number
-- and file format ourselfs
outFileNamePrefix = util.GetParam("-o", "distributed_domain_")

-- We read in the number of time steps to be performed
numTimeSteps = util.GetParamNumber("-numTimeSteps", 10) -- default dimension is 10



-- Now its time to create the domain object. We will use an util method here,
-- which automatically creates the right domain for the given dimension.
dom = util.CreateDomain(dim)

-- Now that we have a domain, we can load a grid into it. We check the return
-- value whether the loading was successful or not.
-- Note that we use the method utilLoadDomain instead of LoadDomain. utilLoadDomain
-- has the benefit that grids are automatically searched in the data/grids folder if
-- they were not found at the default locations (execution-path or a path specified
-- in your environments path-variable).
if util.LoadDomain(dom, gridName) == false then
	print("Loading of domain " .. gridName .. " failed. Aborting.")
--	call exit to leave the application right away.
	exit() 
end

-- Now that we're here, the domain was successfully loaded
print("Loaded domain from " .. gridName)


-- Distribute the domain to all involved processes
if DistributeDomain(dom) == false then
	print("Error while Distributing Domain. Aborting.")
	exit()
end


-- Lets save the domain on each process
outFileName = outFileNamePrefix .. GetProcessRank() .. ".ugx"
if SaveDomain(dom, outFileName) == false then
	print("Saving of domain to " .. outFileName .. " failed. Aborting.")
	exit()
end

-- Everything seems to went fine.
print("Saved domain to " .. outFileName)

-- Check the subset handler if all subsets are given
sh = dom:get_subset_handler()
if sh:get_subset_index("Inner") == -1 then
	print("Domain does not contain subset 'Inner'. Aborting.")
	exit()
end

if sh:get_subset_index("Boundary") == -1 then
	print("Domain does not contain subset 'Boundary'. Aborting.")
	exit()
end


-- We create an Approximation Space
-- which describes the unknowns in the equation. In our case we will
-- only require one unknown for the concentration ("c").
-- Note that the Approximation Space is build on the domain created above.
print("Create ApproximationSpace")
approxSpace = util.CreateApproximationSpace(dom) -- creates new object
approxSpace:add_fct("c", "Lagrange", 1)          -- adds one function
approxSpace:init()                               -- fixes the space


----------------------------------------------------
----------------------------------------------------
-- User function and User Data
----------------------------------------------------
----------------------------------------------------

-- Next we need a lot of User Functions and User Data to specify the 
-- convection-diffusion problem's coefficient. The data can be
-- set from the lua script.

----------------------------------------------------
-- Diffusion Tensor
----------------------------------------------------

-- For the convection-diffusion problem, we need a diffusion tensor.
-- We first set up simple constant diffusion tensors in the lua script.
-- Parameters are the coordinates of the point at which the tensor should be
-- evaluated and the time of the current time-step.
-- Note that we simply return multiple values - the coefficients of the diffusion matrix.
function constDiffTensor1d(x, t)
	return	1
end

function constDiffTensor2d(x, y, t)
	return	1, 0, 
			0, 1
end

function constDiffTensor3d(x, y, z, t)
	return	1, 0, 0,
			0, 1, 0,
			0, 0, 1
end

-- Now we wrap this lua functions into an data-callback, that can be passed to
-- the element discretization later.
luaDiffTensorCallback = util.CreateLuaUserMatrix("constDiffTensor" .. dim .. "d", dim)

-- We can gain the same effect, by using a constant user matrix implementation,
-- since we have not used the coordinates at all. Since a hard-coded, constant
-- evaluation is faster than the lua-callback, we should always used the constant
-- data when possible. We can create a unit-tensor as follows. Note, that this 
-- object can again be passed to the element discretization since it satisfies
-- the same interface as the lua-callback.
constDiffTensor = util.CreateConstDiagUserMatrix(1.0, dim)

-- A third possibility is to use a DataLinker. A Linker simply combines data
-- to some new data and can be used e.g. to combine constant data and data
-- produced by other or the own discretization. All Linker derive from the
-- class DataLinker and a special implementation is made for functions, that
-- can be given by lua-callbacks. Lets use the LuaUserFunctionMatrixNumber
-- here, that gets as an import an scalar value and produces as the output
-- a matrix.
-- First we must set up the lua functions and the derivative w.r.t to the arguments
function linkDiffTensor1d(c)
	return c
end
function linkDiffTensor2d(c)
	local v = 0.001 * c
	return 	v, 0,
			0, v
end
function linkDiffTensor3d(c)
	return 	c, 0, 0,
			0, c, 0,
			0, 0, c
end

function DlinkDiffTensor1d_c(c)
	return 1
end
function DlinkDiffTensor2d_c(c)
	local v = 0.001
	return 	v, 0,
			0, v
end
function DlinkDiffTensor3d_c(c)
	return 	1, 0, 0,
			0, 1, 0,
			0, 0, 1
end

-- Next we create a linker
if 		dim == 1 then linkedDiffTensor = LuaUserFunctionMatrixNumber1d();
elseif 	dim == 2 then linkedDiffTensor = LuaUserFunctionMatrixNumber2d();
elseif 	dim == 3 then linkedDiffTensor = LuaUserFunctionMatrixNumber3d();
else print("Choosen Dimension not supported. Exiting."); exit(); end

-- We pass the lua-callback to the linker for the evaluation of the data. The
-- second argument gives the number of inputs.
linkedDiffTensor:set_lua_value_callback("linkDiffTensor"..dim.."d", 1);

-- Now we have to assign also the derivative w.r.t to the input as an analytical
-- lua function. Since we have only one input, we set the 0'th input derivative.
linkedDiffTensor:set_lua_deriv_callback(0, "DlinkDiffTensor"..dim.."d_c");

-- Finally it remains to specify what the input should be. This is left open here
-- but will be the concentration of our conv-diff equation. We cannot set it here
-- since the element disc has not yet been defined, but we will come back to the
-- point later.

----------------------------------------------------
-- Right-Hand Side
----------------------------------------------------

-- The same for the right hand side ..
function ourRhs1d(x, t)
	return 0.0
end

function ourRhs2d(x, y, t)
	return 0.0
end

function ourRhs3d(x, y, z, t)
	return 0.0
end

rhsCallback = util.CreateLuaUserNumber("ourRhs" .. dim .."d", dim)



-- Now we will create the discretization objects. This is performed in
-- several steps to guarantee maximal flexibility. First we will create
-- element discretizations. Those perform the discretization on the
-- actual elements. UG features several differen discretization schemes.
-- We use the convection / diffusion scheme with finite volumes.
-- Note that we do not set a convection callback, which will result in
-- a default convection of 0 - the convection / diffusion equation thus
-- simplifies to a pure diffusion equation - the laplace problem.

----------------------------------------------------
-- Convection - Diffusion Element Discretization
----------------------------------------------------

-- Here we create a new instance of a convection diffusion equation.
-- We furthermore tell it that it shall operate on the unknowns associated
-- with function "c" (the concentration defined in the approximation space)
-- and that the discretization shall only operate on elements in the subset
-- "Inner". Note that one could specify multiple subsets here by enumerating
-- them all in the subsets-string separated by , (i.e. "Inner1, Inner2"). 
elemDisc = util.CreateFV1ConvDiff(approxSpace, "c", "Inner")
elemDisc:set_upwind_amount(0)


-- Now, the conv-diff assembling has been created. We can request from it the
-- data it can produce. In the case of this equation, this is the concentration
-- and its derivative. We decide to use the concentration value as an input
-- for the linker, that we have set up for the Diffusion matrix above. Since
-- our linker has only one input, we set the 0'th input to the concentration.
linkedDiffTensor:set_input(0, elemDisc:get_concentration())

-- Now we have to choose, which coefficients to use. Out of the created 
-- data objects we can choose. E.g. for the Diffusion matrix, we have 
-- the three choices, created above. We pick one ...
--elemDisc:set_diffusion_tensor(luaDiffTensorCallback)	-- set lua diffusion matrix

-- ... we could also have used these two possibilities
--elemDisc:set_diffusion_tensor(constDiffTensor)	-- set const diffusion matrix
elemDisc:set_diffusion_tensor(linkedDiffTensor)	-- set linker diffusion matrix


-- we add also the other coefficients
elemDisc:set_rhs(rhsCallback)						-- set the right hand side

----------------------------------------------------
-- Dirichlet Boundary
----------------------------------------------------

-- We create some boundary conditions as shown in the preceeding tutorials ...
function ourDirichletBnd1d(x, t)
	return true, 0.0
end

function ourDirichletBnd2d(x, y, t)
	return true, 0.0
end

function ourDirichletBnd3d(x, y, z, t)
	return true, 0.0
end

-- ... and wrap it into a lua-callback
dirichletCallback = util.CreateLuaBoundaryNumber("ourDirichletBnd" .. dim .. "d", dim)

-- lets setup the dirichlet values as explained in the previous tutorials
dirichletBnd = util.CreateDirichletBoundary(approxSpace)
dirichletBnd:add_boundary_value(dirichletCallback, "c", "Boundary")

----------------------------------------------------
-- Adding all Discretizations
----------------------------------------------------

-- Finally we create the discretization object which combines all the
-- separate discretizations into one domain discretization.
domainDisc = DomainDiscretization()
domainDisc:add_elem_disc(elemDisc)
domainDisc:add_post_process(dirichletBnd)

-- Now we create a time discretization. We use the theta-scheme. The time 
-- stepping scheme gets passed the domain discretization and will assemble
-- mass- and stiffness parts using the domain disc, then adding them in a
-- weighted way.
timeDisc = ThetaTimeDiscretization()
timeDisc:set_domain_discretization(domainDisc)
timeDisc:set_theta(0.0) -- 0.0 is backward euler


-- Now we create an operator from the time discretization. We use the 
-- (nonlinear)-Operator interface.
op = AssembledOperator()
op:set_discretization(timeDisc)
op:set_dof_distribution(approxSpace:get_surface_dof_distribution())
op:init()

----------------------------------------------------
----------------------------------------------------
-- Solver setup
----------------------------------------------------
----------------------------------------------------

-- we need a linear solver that solves the linearized problem inside of the
-- newton solver iteration. So, we create an exact LU solver here.
linSolver = LU()

-- Next we need a convergence check, that computes the defect within each 
-- newton step and stops the iteration when a specified creterion is fullfilled.
-- For our purpose is the StandardConvergenceCheck is sufficient. Please note,
-- that this class derives from a general IConvergenceCheck-Interface and
-- also more specialized or self-coded convergence checks could be used.
newtonConvCheck = StandardConvergenceCheck()
newtonConvCheck:set_maximum_steps(10)
newtonConvCheck:set_minimum_defect(5e-8)
newtonConvCheck:set_reduction(1e-10)
newtonConvCheck:set_verbose_level(true)

-- Within each newton step a line search can be applied. In order to do so an
-- implementation of the ILineSearch-Interface can be passed to the newton
-- solver. Here again we use the standard implementation.
newtonLineSearch = StandardLineSearch()

-- Now we can set up the newton solver. We set the linear solver created above
-- as solver for the linearized problem and set the convergence check. If you
-- want to you can also set the line search.
newtonSolver = NewtonSolver()
newtonSolver:set_linear_solver(linSolver)
newtonSolver:set_convergence_check(newtonConvCheck)
--newtonSolver:set_line_search(newtonLineSearch)

-- Finally we set the non-linear operator created above and initiallize the
-- Newton solver for this operator.
newtonSolver:init(op)

----------------------------------------------------
----------------------------------------------------
-- Time loop
----------------------------------------------------
----------------------------------------------------

-- Now we are at the point to initalize the time stepping and start the time
-- loop

-- We create a grid function on the surface of our MultiGrid hierarchy.
u = approxSpace:create_surface_function()

-- Lets chose a fixed time step size and start at time point 0.0. Our first
-- step is step number 0.
dt = 1
time = 0.0
step = 0

-- Next we have to initialize the solution with the start configuration. We
-- interpolate the data given by a lua-callback

-- setup the lua functions ...
function StartValue1d(x, t)
	return x
end
function StartValue2d(x, y, t)
	return 1.0
end
function StartValue3d(x, y, z, t)
	return 1.0
end

-- ... and wrap the lua-callback
LuaStartValue = util.CreateLuaUserNumber("StartValue"..dim.."d", dim)

-- Now interpolate the function
InterpolateFunction(LuaStartValue, u, "c", time);

-- In order to plot our time steps, we need a VTK writer. For time dependent
-- problems we start a time series. This is necessary to group the time 
-- series at the end of the time loop. We write the start solution at the beginning.
out = util.CreateVTKWriter(dim)
out:begin_timeseries("Solution", u)
out:print("Solution", u, step, time)

-- Since we use a one-step scheme, we need one extra solution vector to store
-- the old time step. This is done using the clone method. All previous 
-- solutions (in this case only one) are stored in the "PreviousSolution" 
-- object, that behaves like a queue of fixed size. We push the start solution
-- as the first old time step to our queue
uOld = u:clone()
prevSol = PreviousSolutions()
prevSol:push(uOld, time)

-- Now we can start our time loop 
for step = 1, numTimeSteps do
	print("++++++ TIMESTEP " .. step .. " BEGIN ++++++")

	-- we choose the time step size to be constant here
	do_dt = dt
	
	-- setup time Disc for old solutions and timestep
	timeDisc:prepare_step(prevSol, do_dt)
	
	-- prepare newton solver
	if newtonSolver:prepare(u) == false then 
		print ("Newton solver prepare failed at step "..step.."."); exit(); 
	end 
	
	-- apply newton solver
	if newtonSolver:apply(u) == false then
		 print ("Newton solver apply failed at step "..step.."."); exit();
	end 

	-- compute the new (absolut) time
	time = prevSol:time(0) + do_dt
	
	-- we write the newly computed time step to our time series
	out:print("Solution", u, step, time)
	
	-- get oldest solution
	oldestSol = prevSol:oldest_solution()

	-- copy values into oldest solution (we reuse the memory here)
	VecScaleAssign(oldestSol, 1.0, u)
	
	-- push oldest solutions with new values to front, oldest sol pointer is poped from end
	prevSol:push_discard_oldest(oldestSol, time)

	print("++++++ TIMESTEP " .. step .. "  END ++++++");
end

-- At the end of the time loop, we finish our time loop. This produces a
-- grouping "Solution.pvd" file, that containes all time steps and can be
-- opened by a viewer like Paraview.
out:end_timeseries("Solution", u)


print("")
print("done.")

