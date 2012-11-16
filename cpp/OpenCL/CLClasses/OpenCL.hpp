
#pragma once

#include <CL/cl.hpp>

#include <functional>
#include <memory>
#include <fstream>
#include <map>
#include <exception>
#include <algorithm>
#include <iostream>

class OpenCL
{

#pragma region ENUM

public:

	enum DeviceType
	{
		AnyDevice = CL_DEVICE_TYPE_ALL,
		CPU = CL_DEVICE_TYPE_CPU,
		GPU = CL_DEVICE_TYPE_GPU,
		Accelerator = CL_DEVICE_TYPE_ACCELERATOR,
		Default = CL_DEVICE_TYPE_DEFAULT
	};

	enum Vendor
	{
		AnyVendor,
		NVidia,
		AMD,
		Intel
	};

	enum MemFlags
	{
		Null = 0,
		UseHostMemory = CL_MEM_USE_HOST_PTR,
		CopyHostMemory = CL_MEM_COPY_HOST_PTR,
		AllocateHostMemory = CL_MEM_ALLOC_HOST_PTR,
		ReadWrite = CL_MEM_READ_WRITE,
		WriteOnly = CL_MEM_WRITE_ONLY,
		ReadOnly = CL_MEM_READ_ONLY
	};

	enum BlockMode
	{
		Blocking = CL_TRUE,
		NotBlocking = CL_FALSE
	};

	enum QueueCommand
	{
		BufferWrite,
		BufferRead
	};

#pragma endregion

#pragma region ATTRIBUTES

private:
	std::auto_ptr<std::vector<cl::Platform>> platforms;
	std::auto_ptr<std::vector<cl::Device>> devices;
	std::auto_ptr<cl_context_properties> cps;
	std::auto_ptr<cl::Context> context;
	std::auto_ptr<cl::CommandQueue> queue;
	std::map<std::string, std::auto_ptr<cl::Program::Sources>> kernelSources;
	std::map<std::string, std::auto_ptr<cl::Program>> programs;
	std::map<std::string, std::auto_ptr<cl::Kernel>> kernels;
	std::map<Vendor, std::string> VendorNames;

	cl::Platform *ChosenPlatform;
	cl_int error;

#pragma endregion

#pragma region CONSTRUCTORS/DESTRUCTOR

public:
	OpenCL()
	{}

#pragma endregion

#pragma region GETTERS

	inline cl::Context const &GetContext() const
	{
		return (*context);
	}

	inline cl::CommandQueue const &GetCommandQueue() const
	{
		return (*queue);
	}

	inline std::vector<cl::Platform> const &GetPlatforms() const
	{
		return (*platforms);
	}

	inline std::vector<cl::Device> const &GetDevices() const
	{
		return (*devices);
	}

#pragma endregion

#pragma region FUNCTIONS

	void InitContext(DeviceType PreferedDevice = AnyDevice, Vendor PreferedVendor = AnyVendor, cl_command_queue_properties commandQueueProperties = 0)
	{
		VendorNames[AMD] = "Advanced Micro Devices";
		VendorNames[NVidia] = "NVIDIA";
		VendorNames[Intel] = "Intel";
		platforms.reset(new std::vector<cl::Platform>());
		cps.reset(new cl_context_properties[3]);
		devices.reset(new std::vector<cl::Device>());
		
		ChosenPlatform = &(GetPreferedPlatform(PreferedDevice, PreferedVendor));
		cps.get()[0] = CL_CONTEXT_PLATFORM;
		cps.get()[1] = (cl_context_properties)(*ChosenPlatform)();
		cps.get()[2] = 0;

		context.reset(new cl::Context(PreferedDevice, cps.get()));
		queue.reset(new cl::CommandQueue(*context, (*devices.get())[0], commandQueueProperties, &error));
		if (error != CL_SUCCESS)
			throw Exception(error, "InitContext");
	}

	void DumpPlatformInfo()
	{
		std::cout	<< "Platform name :\t\t" << (*ChosenPlatform).getInfo<CL_PLATFORM_NAME>() << "\n"
					<< "       - vendor :\t" << (*ChosenPlatform).getInfo<CL_PLATFORM_VENDOR>() << "\n"
					<< "       - version :\t" << (*ChosenPlatform).getInfo<CL_PLATFORM_VERSION>() << "\n"
					<< "       - profile :\t" << (*ChosenPlatform).getInfo<CL_PLATFORM_PROFILE>() << "\n"
					<< "\nExtensions :\n" << (*ChosenPlatform).getInfo<CL_PLATFORM_EXTENSIONS>() << "\n";
	}

#pragma region KERNEL FUNCTIONS

	void AddKernelSource(std::string const &kernelFile)
	{
		std::fstream sourceFile(kernelFile, std::fstream::out | std::fstream::in);
		std::string kernelCode(std::istreambuf_iterator<char>(sourceFile),
							  (std::istreambuf_iterator<char>()));
		cl::Program::Sources *src = new cl::Program::Sources(1);
		(*src)[0] = std::make_pair(kernelCode.c_str(), kernelCode.length() + 1);
		programs[kernelFile].reset(new cl::Program(*context, *src, &error));
		if (error != CL_SUCCESS || (error = programs[kernelFile]->build(*devices)) != CL_SUCCESS)
		{
			std::cout << std::endl << programs[kernelFile]->getBuildInfo<CL_PROGRAM_BUILD_LOG>((*devices)[0]) << std::endl;
			throw Exception(error, "AddKernelSource");
		}
	}

	void AddKernel(std::string const &kernelFile, std::string const &kernelName)
	{
		kernels[kernelName].reset(new cl::Kernel(*(programs[kernelFile].get()), kernelName.c_str(), &error));
		if (error != CL_SUCCESS)
			throw Exception(error, "AddKernel");
	}

	template <typename T>
	void SetKernelArgument(std::string const &kernelName, unsigned int index, T val)
	{
		if ((error = kernels[kernelName]->setArg(index, val)) != CL_SUCCESS)
			throw Exception(error, "SetKernelArgument");
	}

	template <typename T>
	void SetKernelArgument(std::string const &kernelName, std::vector<T> val)
	{
		for (unsigned int i = 0, i < val.size(); ++i)
			if ((error = kernels[kernelName]->setArg(i, val[i])) != CL_SUCCESS)
				throw Exception(error, "SetKernelArgument");
	}

#pragma endregion

#pragma region COMMANDQUEUE FUNCTIONS

	inline void EnqueueKernel(std::string const &kernelName, cl::NDRange const &offset, cl::NDRange const &global, cl::NDRange const &local, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueNDRangeKernel((*kernels[kernelName]), offset, global, local, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueKernel");
	}

	inline void EnqueueKernel(std::string const &kernelName, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueTask((*kernels[kernelName]), events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueKernel");
	}

	inline void EnqueueKernel(void (__stdcall*nativeKernel)(void*), std::pair<void*, ::size_t> args, std::vector<cl::Memory> const *memObject = nullptr, std::vector<void const*> const *memLocations = nullptr, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueNativeKernel(nativeKernel, args, memObject, memLocations, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueKernel");
	}

	inline void EnqueueWrite(cl::Buffer const &buffer, BlockMode mode, ::size_t offset, ::size_t buffSize, void const *data, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueWriteBuffer(buffer, mode, offset, buffSize, data, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueWrite");
	}

	inline void EnqueueWrite(cl::Image const &image, BlockMode mode, cl::size_t<3> const &origin, cl::size_t<3> const &region, ::size_t row, ::size_t pitch, void *data, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueWriteImage(image, mode, origin, region, row, pitch, data, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueWrite");
	}

	inline void EnqueueRead(cl::Buffer const &buffer, BlockMode mode, ::size_t offset, ::size_t buffSize, void *data, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueReadBuffer(buffer, mode, offset, buffSize, data, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueRead");
	}

	inline void EnqueueRead(cl::Image const &image, BlockMode mode, cl::size_t<3> const &origin, cl::size_t<3> const &region, ::size_t row, ::size_t pitch, void *data, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueWriteImage(image, mode, origin, region, row, pitch, data, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueRead");
	}

	inline void EnqueueCopy(cl::Buffer const &source, cl::Buffer const &destination, ::size_t sourceOffset, ::size_t destOffset, ::size_t size, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueCopyBuffer(source, destination, sourceOffset, destOffset, size, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueCopy");
	}

	inline void EnqueueCopy(cl::Image const &source, cl::Image const &destination, cl::size_t<3> const &srcOrigin, cl::size_t<3> const &dstOrigin, cl::size_t<3> const &region, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueCopyImage(source, destination, srcOrigin, dstOrigin, region, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueCopy");
	}

	inline void EnqueueCopy(cl::Image const &src, cl::Buffer const &dst, cl::size_t<3> const &srcOrigin, cl::size_t<3> const &region, ::size_t dstOffset, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueCopyImageToBuffer(src, dst, srcOrigin, region, dstOffset, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueCopy");
	}

	inline void EnqueueCopy(cl::Buffer const &source, cl::Image const &destination, ::size_t srcOffset, cl::size_t<3> const &dstOrigin, cl::size_t<3> const &region, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueCopyBufferToImage(source, destination, srcOffset, dstOrigin, region, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueCopy");
	}

	inline void *EnqueueMap(cl::Buffer const &buffer, BlockMode mode, cl_map_flags flags, ::size_t offset, ::size_t size, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		void *ret = queue->enqueueMapBuffer(buffer, mode, flags, offset, size, events, event, &error);
		if (error != CL_SUCCESS)
			throw Exception(error, "EnqueueMap");
		return ret;
	}

	inline void *EnqueueMap(cl::Image const &image, BlockMode mode, cl_map_flags flags, cl::size_t<3> &origin, cl::size_t<3> &region, ::size_t *rowPitch, ::size_t *slicePitch, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		void *ret = queue->enqueueMapImage(image, mode, flags, origin, region, rowPitch, slicePitch, events, event, &error);
		if (error != CL_SUCCESS)
			throw Exception(error, "EnqueueMap");
	}

	inline void EnqueueUnmap(cl::Memory const &buffer, void *mappedData, std::vector<cl::Event> const *events = nullptr, cl::Event *event = nullptr)
	{
		if ((error = queue->enqueueUnmapMemObject(buffer, mappedData, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueUnmap");
	}

	inline void EnqueueMarker(cl::Event *event)
	{
		if ((error = queue->enqueueMarker(event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueMarker");
	}

	inline void EnqueueWait(std::vector<cl::Event> const &events)
	{
		if ((error = queue->enqueueWaitForEvents(events)) != CL_SUCCESS)
			throw Exception(error, "EnqueueWait");
	}

	inline void EnqueueBarrier(void)
	{
		if ((error = queue->enqueueBarrier()) != CL_SUCCESS)
			throw Exception(error, "EnqueueBarrier");
	}

	inline void FlushQueue(void)
	{
		if ((error = queue->flush()) != CL_SUCCESS)
			throw Exception(error, "FlushQueue");
	}

	inline void FinishQueue(void)
	{
		if ((error = queue->finish()) != CL_SUCCESS && error != CL_INVALID_COMMAND_QUEUE)
			throw Exception(error, "FinishQueue");
	}

#pragma endregion

#pragma region CONTAINER FUNCTIONS

	cl::Buffer *CreateBuffer(MemFlags flags, unsigned int buffSize, void *data = nullptr)
	{
		cl::Buffer *newBuffer = new cl::Buffer(*context, flags, buffSize, data, &error);

		if (error != CL_SUCCESS)
		{
			delete newBuffer;
			throw Exception(error, "CreateBuffer");
		}
		return (newBuffer);
	}

	cl::Image2D *CreateImage2D(MemFlags flags, cl::ImageFormat format, unsigned int width, unsigned int height, unsigned int rowPitch = 0, void *data = nullptr)
	{
		cl::Image2D *newImage = new cl::Image2D(*context, flags, format, width, height, rowPitch, data, &error);

		if (error != CL_SUCCESS)
		{
			delete newImage;
			throw Exception(error, "CreateImage2D");
		}
		return (newImage);
	}

	cl::Image3D *CreateImage3D(MemFlags flags, cl::ImageFormat format, unsigned int width, unsigned int height, unsigned int depth, unsigned int rowPitch = 0, unsigned int slicePitch = 0, void *data = nullptr)
	{
		cl::Image3D *newImage = new cl::Image3D(*context, flags, format, width, height, depth, rowPitch, slicePitch, data, &error);
		
		if (error != CL_SUCCESS)
		{
			delete newImage;
			throw Exception(error, "CreateImage3D");
		}
		return (newImage);
	}

	cl::Sampler *CreateSampler(bool isNormalized, cl_addressing_mode addressing_mode, cl_filter_mode filter_mode)
	{
		cl::Sampler *newSampler = new cl::Sampler(*context, isNormalized, addressing_mode, filter_mode, &error);

		if (error != CL_SUCCESS)
		{
			delete newSampler;
			throw Exception(error, "CreateSampler");
		}
		return (newSampler);
	}

#pragma endregion

#pragma region PRIVATE FUNCTIONS

private:
	cl::Platform &GetPreferedPlatform(DeviceType PreferedDevice, Vendor PreferedVendor)
	{
		if ((error = cl::Platform::get(platforms.get())) != CL_SUCCESS)
			throw Exception(error, "GetPreferedPlatform");
		if (platforms->size() == 0)
			throw Exception("No OpenCL platforms found", "GetPreferedPlatform");

		bool vendorAny = false;
		int PlatformId = -1;
		int idx = -1;
		if (PreferedVendor == AnyVendor)
			vendorAny = true;
		for (cl::Platform platf: *platforms)
		{
			if (vendorAny || (platf.getInfo<CL_PLATFORM_VENDOR>().find(VendorNames[PreferedVendor]) != std::string::npos))
			{
				++idx;
				if (platf.getDevices(PreferedDevice, devices.get()) != CL_SUCCESS)
					continue;
				PlatformId = idx;
				break;
			}
		}
		if (PlatformId == -1)
			throw Exception("Could not find OpenCL compatible platform", "GetPreferedPlatform");
		return ((*platforms)[PlatformId]);
	}

#pragma endregion

#pragma endregion

#pragma region EXCEPTION

public:
	class Exception : std::exception
	{
	private:
		cl_int errCode;
		std::string function;
		std::string errorMsg;

	public:
		Exception(cl_int error, std::string const &func) throw()
			:errCode(error), function(func), errorMsg("")
		{}

		Exception(std::string const &errMsg, std::string const &func)
			:errCode(1), function(func), errorMsg(errMsg)
		{}

		virtual ~Exception() throw()
		{}

		virtual std::string const what() throw()
		{
			if (errCode <= 0)
			{
				switch (errCode)
				{
				case CL_SUCCESS:								return function + " : Success!";
				case CL_DEVICE_NOT_FOUND:						return function + " : Device not found.";
				case CL_DEVICE_NOT_AVAILABLE:					return function + " : Device not available";
				case CL_COMPILER_NOT_AVAILABLE:					return function + " : Compiler not available";
				case CL_MEM_OBJECT_ALLOCATION_FAILURE:			return function + " : Memory object allocation failure";
				case CL_OUT_OF_RESOURCES:						return function + " : Out of resources";
				case CL_OUT_OF_HOST_MEMORY:						return function + " : Out of host memory";
				case CL_PROFILING_INFO_NOT_AVAILABLE:			return function + " : Profiling information not available";
				case CL_MEM_COPY_OVERLAP:						return function + " : Memory copy overlap";
				case CL_IMAGE_FORMAT_MISMATCH:					return function + " : Image format mismatch";
				case CL_IMAGE_FORMAT_NOT_SUPPORTED:				return function + " : Image format not supported";
				case CL_BUILD_PROGRAM_FAILURE:					return function + " : Program build failure";
				case CL_MAP_FAILURE:							return function + " : Map failure";
				case CL_INVALID_VALUE:							return function + " : Invalid value";
				case CL_INVALID_DEVICE_TYPE:					return function + " : Invalid device type";
				case CL_INVALID_PLATFORM:						return function + " : Invalid platform";
				case CL_INVALID_DEVICE:							return function + " : Invalid device";
				case CL_INVALID_CONTEXT:						return function + " : Invalid context";
				case CL_INVALID_QUEUE_PROPERTIES:				return function + " : Invalid queue properties";
				case CL_INVALID_COMMAND_QUEUE:					return function + " : Invalid command queue";
				case CL_INVALID_HOST_PTR:						return function + " : Invalid host pointer";
				case CL_INVALID_MEM_OBJECT:						return function + " : Invalid memory object";
				case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:		return function + " : Invalid image format descriptor";
				case CL_INVALID_IMAGE_SIZE:						return function + " : Invalid image size";
				case CL_INVALID_SAMPLER:						return function + " : Invalid sampler";
				case CL_INVALID_BINARY:							return function + " : Invalid binary";
				case CL_INVALID_BUILD_OPTIONS:					return function + " : Invalid build options";
				case CL_INVALID_PROGRAM:						return function + " : Invalid program";
				case CL_INVALID_PROGRAM_EXECUTABLE:				return function + " : Invalid program executable";
				case CL_INVALID_KERNEL_NAME:					return function + " : Invalid kernel name";
				case CL_INVALID_KERNEL_DEFINITION:				return function + " : Invalid kernel definition";
				case CL_INVALID_KERNEL:							return function + " : Invalid kernel";
				case CL_INVALID_ARG_INDEX:						return function + " : Invalid argument index";
				case CL_INVALID_ARG_VALUE:						return function + " : Invalid argument value";
				case CL_INVALID_ARG_SIZE:						return function + " : Invalid argument size";
				case CL_INVALID_KERNEL_ARGS:					return function + " : Invalid kernel arguments";
				case CL_INVALID_WORK_DIMENSION:					return function + " : Invalid work dimension";
				case CL_INVALID_WORK_GROUP_SIZE:				return function + " : Invalid work group size";
				case CL_INVALID_WORK_ITEM_SIZE:					return function + " : Invalid work item size";
				case CL_INVALID_GLOBAL_OFFSET:					return function + " : Invalid global offset";
				case CL_INVALID_EVENT_WAIT_LIST:				return function + " : Invalid event wait list";
				case CL_INVALID_EVENT:							return function + " : Invalid event";
				case CL_INVALID_OPERATION:						return function + " : Invalid operation";
				case CL_INVALID_GL_OBJECT:						return function + " : Invalid OpenGL object";
				case CL_INVALID_BUFFER_SIZE:					return function + " : Invalid buffer size";
				case CL_INVALID_MIP_LEVEL:						return function + " : Invalid mip-map level";
				default:										return function + " : Unknown";
				}
			}
			else
				return (function + " : " + errorMsg);
		}
	};
#pragma endregion
};