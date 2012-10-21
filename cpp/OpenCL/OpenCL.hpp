
#pragma once

#define __CL_ENABLE_EXCEPTIONS

#include "cl.hpp"
#include <memory>
#include <fstream>
#include <map>
#include <exception>
#include <algorithm>

using namespace cl;

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

	enum MemMode
	{
		ReadOnly = CL_MEM_READ_ONLY,
		WriteOnly = CL_MEM_WRITE_ONLY,
		ReadWrite = CL_MEM_READ_WRITE
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
	std::auto_ptr<VECTOR_CLASS<Platform>> platforms;
	std::auto_ptr<VECTOR_CLASS<Device>> devices;
	std::auto_ptr<cl_context_properties> cps;
	std::auto_ptr<Context> context;
	std::auto_ptr<CommandQueue> queue;
	std::map<std::string, std::auto_ptr<Program::Sources>> kernelSources;
	std::map<std::string, std::auto_ptr<Program>> programs;
	std::map<std::string, std::auto_ptr<Kernel>> kernels;
	std::map<Vendor, std::string> VendorNames;

	Platform *ChosenPlatform;
	cl_int error;

#pragma endregion

#pragma region CONSTRUCTORS/DESTRUCTOR

public:
	OpenCL()
	{}

#pragma endregion

#pragma region GETTERS

	inline Context const &GetContext() const
	{
		return (*context);
	}

	inline CommandQueue const &GetCommandQueue() const
	{
		return (*queue);
	}

	inline VECTOR_CLASS<Platform> const &GetPlatforms() const
	{
		return (*platforms);
	}

	inline VECTOR_CLASS<Device> const &GetDevices() const
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
		platforms.reset(new VECTOR_CLASS<Platform>());
		cps.reset(new cl_context_properties[3]);
		
		ChosenPlatform = &(GetPreferedPlatform(PreferedDevice, PreferedVendor));
		cps.get()[0] = CL_CONTEXT_PLATFORM;
		cps.get()[1] = (cl_context_properties)(*ChosenPlatform)();
		cps.get()[2] = 0;

		context.reset(new Context(PreferedDevice, cps.get()));
		queue.reset(new CommandQueue(*context, (*devices.get())[0], commandQueueProperties, &error));
		if (error != CL_SUCCESS)
			throw Exception(error, "InitContext");
	}

#pragma region KERNEL FUNCTIONS

	void AddKernelSource(std::string const &kernelFile)
	{
		std::fstream sourceFile(kernelFile, std::fstream::out | std::fstream::in);
		std::string kernelCode(std::istreambuf_iterator<char>(sourceFile),
							  (std::istreambuf_iterator<char>()));
		kernelSources[kernelFile].reset(new Program::Sources(1,
										std::make_pair(kernelCode.c_str(),
										kernelCode.length() + 1)));
		programs[kernelFile].reset(new Program(*context, *kernelSources[kernelFile]));
		if ((error = programs[kernelFile]->build(*devices)) != CL_SUCCESS)
			throw Exception(error, "AddKernelSource");
	}

	void AddKernel(std::string const &kernelFile, std::string const &kernelName)
	{
		kernels[kernelFile].reset(new Kernel(*(programs[kernelFile].get()), kernelName.c_str(), &error));
		if (error != CL_SUCCESS)
			throw Exception(error, "AddKernel");
	}

	template <typename T>
	void SetKernelArgument(std::string const &kernelName, unsigned int index, T val)
	{
		if ((error = kernels[kernelName]->setArg(index, val) != CL_SUCCESS)
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

	inline void EnqueueKernel(std::string const &kernelName, NDRange const &offset, NDRange const &global, NDRange const &local, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueNDRangeKernel((*kernels[kernelName]), offset, global, local, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueKernel");
	}

	inline void EnqueueKernel(std::string const &kernelName, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueTask((*kernels[kernelName]), events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueKernel");
	}

	inline void EnqueueKernel(void (*nativeKernel)(void*), std::pair<void*, ::size_t> args, VECTOR_CLASS<Memory> const *memObject = nullptr, VECTOR_CLASS<void const*> const *memLocations = nullptr, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueNativeKernel(nativeKernel, args, memObject, memLocations, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueKernel");
	}

	inline void EnqueueWrite(Buffer const &buffer, BlockMode mode, ::size_t offset, ::size_t buffSize, void const *data, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueWriteBuffer(buffer, mode, offset, buffSize, data, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueWrite");
	}

	inline void EnqueueWrite(Image const &image, BlockMode mode, cl::size_t<3> const &origin, cl::size_t<3> const &region, ::size_t row, ::size_t pitch, void *data, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueWriteImage(image, mode, origin, region, row, pitch, data, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueWrite");
	}

	inline void EnqueueRead(Buffer const &buffer, BlockMode mode, ::size_t offset, ::size_t buffSize, void *data, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueReadBuffer(buffer, mode, offset, buffSize, data, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueRead");
	}

	inline void EnqueueRead(Image const &image, BlockMode mode, cl::size_t<3> const &origin, cl::size_t<3> const &region, ::size_t row, ::size_t pitch, void *data, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueWriteImage(image, mode, origin, region, row, pitch, data, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueRead");
	}

	inline void EnqueueCopy(Buffer const &source, Buffer const &destination, ::size_t sourceOffset, ::size_t destOffset, ::size_t size, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueCopyBuffer(source, destination, sourceOffset, destOffset, size, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueCopy");
	}

	inline void EnqueueCopy(Image const &source, Image const &destination, cl::size_t<3> const &srcOrigin, cl::size_t<3> const &dstOrigin, cl::size_t<3> const &region, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueCopyImage(source, destination, srcOrigin, dstOrigin, region, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueCopy");
	}

	inline void EnqueueCopy(Image const &src, Buffer const &dst, cl::size_t<3> const &srcOrigin, cl::size_t<3> const &region, ::size_t dstOffset, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueCopyImageToBuffer(src, dst, srcOrigin, region, dstOffset, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueCopy");
	}

	inline void EnqueueCopy(Buffer const &source, Image const &destination, ::size_t srcOffset, cl::size_t<3> const &dstOrigin, cl::size_t<3> const &region, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueCopyBufferToImage(source, destination, srcOffset, dstOrigin, region, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueCopy");
	}

	inline void *EnqueueMap(Buffer const &buffer, BlockMode mode, cl_map_flags flags, ::size_t offset, ::size_t size, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		void *ret = queue->enqueueMapBuffer(buffer, mode, flags, offset, size, events, event, &error);
		if (error != CL_SUCCESS)
			throw Exception(error, "EnqueueMap");
		return ret;
	}

	inline void *EnqueueMap(Image const &image, BlockMode mode, cl_map_flags flags, cl::size_t<3> &origin, cl::size_t<3> &region, ::size_t *rowPitch, ::size_t *slicePitch, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		void *ret = queue->enqueueMapImage(image, mode, flags, origin, region, rowPitch, slicePitch, events, event, &error);
		if (error != CL_SUCCESS)
			throw Exception(error, "EnqueueMap");
	}

	inline void EnqueueUnmap(Memory const &buffer, void *mappedData, VECTOR_CLASS<Event> const *events = nullptr, Event *event = nullptr)
	{
		if ((error = queue->enqueueUnmapMemObject(buffer, mappedData, events, event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueUnmap");
	}

	inline void EnqueueMarker(Event *event)
	{
		if ((error = queue->enqueueMarker(event)) != CL_SUCCESS)
			throw Exception(error, "EnqueueMarker");
	}

	inline void EnqueueWait(VECTOR_CLASS<Event> const &events)
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
		if ((error = queue->finish()) != CL_SUCCESS)
			throw Exception(error, "FinishQueue");
	}

#pragma endregion

#pragma region CONTAINER FUNCTIONS

	Buffer *CreateBuffer(MemMode mode, unsigned int buffSize, void *data = nullptr)
	{
		Buffer *newBuffer = new Buffer(*context, mode, buffSize, data, &error);

		if (error != CL_SUCCESS)
		{
			delete newBuffer;
			throw Exception(error, "CreateBuffer");
		}
		return (newBuffer);
	}

	Image2D *CreateImage2D(cl_mem_flags flags, ImageFormat format, unsigned int width, unsigned int height, unsigned int rowPitch = 0, void *data = nullptr)
	{
		Image2D *newImage = new Image2D(*context, flags, format, width, height, rowPitch, data, &error);

		if (error != CL_SUCCESS)
		{
			delete newImage;
			throw Exception(error, "CreateImage2D");
		}
		return (newImage);
	}

	Image3D *CreateImage3D(cl_mem_flags flags, ImageFormat format, unsigned int width, unsigned int height, unsigned int depth, unsigned int rowPitch = 0, unsigned int slicePitch = 0, void *data = nullptr)
	{
		Image3D *newImage = new Image3D(*context, flags, format, width, height, depth, rowPitch, slicePitch, data, &error);
		
		if (error != CL_SUCCESS)
		{
			delete newImage;
			throw Exception(error, "CreateImage3D");
		}
		return (newImage);
	}

	Sampler *CreateSampler(bool isNormalized, cl_addressing_mode addressing_mode, cl_filter_mode filter_mode)
	{
		Sampler *newSampler = new Sampler(*context, isNormalized, addressing_mode, filter_mode, &error);

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
	Platform &GetPreferedPlatform(DeviceType PreferedDevice, Vendor PreferedVendor)
	{
		if ((error = Platform::get(platforms.get())) != CL_SUCCESS)
			throw Exception(error, "GetPreferedPlatform");
		if (platforms->size() == 0)
			throw Exception("No OpenCL platforms found", "GetPreferedPlatform");

		bool vendorAny = false;
		int PlatformId = -1;
		int idx = -1;
		if (PreferedVendor == AnyVendor)
			vendorAny = true;
		for (Platform platf: *platforms)
		{
			if (vendorAny || platf.getInfo<CL_PLATFORM_VENDOR>().find(VendorNames[PreferedVendor]) != std::string::npos)
			{
				try
				{
					++idx;
					platf.getDevices(PreferedDevice, devices.get());
					PlatformId = idx;
					break;
				}
				catch (Error *e) {}
			}
		}
		if (PlatformId == -1)
			throw Exception("Could not find OpenCL compatible platform", "GetPreferedPlatform");
		return ((*platforms)[PlatformId]);
	}

#pragma endregion

#pragma endregion

#pragma region EXCEPTION

	class Exception : std::exception
	{
	private:
		cl_uint errCode;
		std::string function;
		std::string errorMsg;

	public:
		Exception(cl_uint error, std::string const &func) throw()
			:errCode(error), function(func), errorMsg("")
		{}

		Exception(std::string const &errMsg, std::string const &func)
			:errCode(1), function(func), errorMsg(errorMsg)
		{}

		virtual ~Exception() throw()
		{}

		virtual std::string const &what() throw()
		{
			if (errCode <= 0)
			{
				switch (errCode)
				{
					case CL_SUCCESS:                            return "Success!";
					case CL_DEVICE_NOT_FOUND:                   return "Device not found.";
					case CL_DEVICE_NOT_AVAILABLE:               return "Device not available";
					case CL_COMPILER_NOT_AVAILABLE:             return "Compiler not available";
					case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "Memory object allocation failure";
					case CL_OUT_OF_RESOURCES:                   return "Out of resources";
					case CL_OUT_OF_HOST_MEMORY:                 return "Out of host memory";
					case CL_PROFILING_INFO_NOT_AVAILABLE:       return "Profiling information not available";
					case CL_MEM_COPY_OVERLAP:                   return "Memory copy overlap";
					case CL_IMAGE_FORMAT_MISMATCH:              return "Image format mismatch";
					case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "Image format not supported";
					case CL_BUILD_PROGRAM_FAILURE:              return "Program build failure";
					case CL_MAP_FAILURE:                        return "Map failure";
					case CL_INVALID_VALUE:                      return "Invalid value";
					case CL_INVALID_DEVICE_TYPE:                return "Invalid device type";
					case CL_INVALID_PLATFORM:                   return "Invalid platform";
					case CL_INVALID_DEVICE:                     return "Invalid device";
					case CL_INVALID_CONTEXT:                    return "Invalid context";
					case CL_INVALID_QUEUE_PROPERTIES:           return "Invalid queue properties";
					case CL_INVALID_COMMAND_QUEUE:              return "Invalid command queue";
					case CL_INVALID_HOST_PTR:                   return "Invalid host pointer";
					case CL_INVALID_MEM_OBJECT:                 return "Invalid memory object";
					case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "Invalid image format descriptor";
					case CL_INVALID_IMAGE_SIZE:                 return "Invalid image size";
					case CL_INVALID_SAMPLER:                    return "Invalid sampler";
					case CL_INVALID_BINARY:                     return "Invalid binary";
					case CL_INVALID_BUILD_OPTIONS:              return "Invalid build options";
					case CL_INVALID_PROGRAM:                    return "Invalid program";
					case CL_INVALID_PROGRAM_EXECUTABLE:         return "Invalid program executable";
					case CL_INVALID_KERNEL_NAME:                return "Invalid kernel name";
					case CL_INVALID_KERNEL_DEFINITION:          return "Invalid kernel definition";
					case CL_INVALID_KERNEL:                     return "Invalid kernel";
					case CL_INVALID_ARG_INDEX:                  return "Invalid argument index";
					case CL_INVALID_ARG_VALUE:                  return "Invalid argument value";
					case CL_INVALID_ARG_SIZE:                   return "Invalid argument size";
					case CL_INVALID_KERNEL_ARGS:                return "Invalid kernel arguments";
					case CL_INVALID_WORK_DIMENSION:             return "Invalid work dimension";
					case CL_INVALID_WORK_GROUP_SIZE:            return "Invalid work group size";
					case CL_INVALID_WORK_ITEM_SIZE:             return "Invalid work item size";
					case CL_INVALID_GLOBAL_OFFSET:              return "Invalid global offset";
					case CL_INVALID_EVENT_WAIT_LIST:            return "Invalid event wait list";
					case CL_INVALID_EVENT:                      return "Invalid event";
					case CL_INVALID_OPERATION:                  return "Invalid operation";
					case CL_INVALID_GL_OBJECT:                  return "Invalid OpenGL object";
					case CL_INVALID_BUFFER_SIZE:                return "Invalid buffer size";
					case CL_INVALID_MIP_LEVEL:                  return "Invalid mip-map level";
					default:									return "Unknown";
				}
			}
			else
				return (function + ": " + errorMsg);
		}
	};
#pragma endregion

};