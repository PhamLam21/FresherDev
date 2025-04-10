/*******************************************************************************
*
*				AWE Core OS API 
*				---------------
*
********************************************************************************
*	  AWECoreOS.h
********************************************************************************
*
*	  Description:	The AWE Core OS API header file.  
*
*	  Copyright:	(c) 2007-2021 DSP Concepts, Inc. All rights reserved.
*					3235 Kifer Road
*					Santa Clara, CA 95054
*
*******************************************************************************/

/**
* @file 
* @brief The AWE Core OS API header file. 
*/

#ifndef AWECOREOS_H_
#define AWECOREOS_H_

#include "StandardDefs.h"
#include "stdio.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "Errors.h"
#include <sched.h>
#include "ProxyIDs.h"
#include <semaphore.h>

#ifdef __cplusplus
extern "C"
{
#endif


/** 
* \defgroup AWECoreOSTypes AWECoreOS Types
* @{
*/

	/** AWE Core OS internal tuning interface logging verbosity level: low -- log only errors */
	#define TUNING_LOG_ERROR	1	
	/** AWE Core OS internal tuning interface logging verbosity level: medium -- log errors and header info from all packets sent and received */	
	#define TUNING_LOG_INFO		2		
	/** AWE Core OS internal tuning interface logging verbosity level: high --log errors, header info and raw packet content for all packets */
	#define TUNING_LOG_DATA		3	


	/** Audio recording notification status type */
	typedef enum {ERROR, INPUT_OVERRUN, OUTPUT_OVERRUN} STATUS;

	/** Audio recording notification callback argument structure */
	typedef struct AWEOSAudioRecordNotification
	{
		STATUS notificationStatus;					/**< type of notification being reported */
		INT32 error;								/**< error value - 0 if xrun */		
		UINT32 xruns;								/**< total xruns on recording stream - 0 if error */
		FLOAT32 xrunTime;							/**< time in seconds of xrun in recording - 0 if error */		
	} AWEOSAudioRecordNotification_t;

	/** Audio recording notification callback type */
	typedef void (*recordNotificationCallbackFunction)(AWEOSAudioRecordNotification_t*);

	/** Audio recording: Notify only on errors */
	#define AUDIO_RECORDING_NOTIFICATION_ERRORS 	(1 << 0)		
	/** Audio recording: Notify only on overruns */
	#define AUDIO_RECORDING_NOTIFICATION_XRUNS 		(1 << 1)
	/** Audio recording: Notify on errors and overruns */
	#define AUDIO_RECORDING_NOTIFICATION_ALL 		(AUDIO_RECORDING_NOTIFICATION_ERRORS | AUDIO_RECORDING_NOTIFICATION_XRUNS)	

	/** Internal threading PID structure, meant to be used with @ref aweOS_getThreadPIDs */
	typedef struct AWEOSThreadPIDs  
	{
		UINT32 workThreadPID; /**< PID of the workThread (the main "heartbeat" of AWECoreOS) */
		UINT32 socketThreadPID; /**< PID of the internally created socket (0 if socket is not created). See @ref aweOS_tuningSocketOpen */
		UINT32 numPumpThreads;  /**< The number of running pump threads. Note: In Low Latency mode, the userspace audiocallback (base layout pump) is not included in this count . */
		UINT32 *pumpThreadPIDs; /**< Pointer to an array of the running pump thread PIDs of size pumpThreadPIDs[numPumpThreads]*/
	} AWEOSThreadPIDs_t;

	/** Versioning structure returned by @ref aweOS_getVersion */
	typedef struct AWEOSVersionInfo
	{
		
		INT32 tuningVer;					/**< Tuning version */ 
		char majorVer;						/**< Major API version (single letter) */
		INT32 minorVer;						/**< Minor API version */
		INT32 procVer;						/**< Processor specific version */
		INT32 buildNumber;					/**< Library build number */	
		const char * textVer;				/**< String of form: "AWECoreOS Version 8.A.1.1 -- Build Number 1234" */
	} AWEOSVersionInfo_t;


	/** The AWE Core OS Instance instance type. The instance itself is opaque, and it is configured with @ref aweOS_getParamDefaults and @ref aweOS_init. This handle will be passed to most API functions.
	* Multi Instance integrations can utilize an array of AWEOSInstances (see the MultiInstance example app) */
	typedef void AWEOSInstance;

	/**
	@brief AWEOSConfigParameters. The AWE Core OS Configuration Parameter Structure
	@details This structure will hold the config parameters for the AWE Core OS instance. 
	* A user must create one of these per AWEOSInstance, and either populate it in manually, or call the @ref aweOS_getParamDefaults to populate it with defaults.
	* Once the structure has been configured, it is passed in to the @ref aweOS_init function with an AWEOSInstance, and the instance is initialized with the given configuration parameters.
	* The heap pointers, pPacketBuffer and pReplyBuffer will be assigned to NULL when calling aweOS_getParamDefaults. If they are not overwritten by user, then memory is allocated in aweOS_init.
	* Internally allocated heap sizes can be controlled via the heapSize configuration parameters. */
	
	typedef struct AWEOSConfigParameters
	{
		
		UINT32 *pFastHeapA; 							/**< Pointer to fast heap A. Allocated and assigned in aweOS_init by default */
		UINT32 *pFastHeapB; 							/**< Pointer to fast heap B. Allocated and assigned in aweOS_init by default */
		UINT32 *pSlowHeap; 								/**< Pointer to slow heap. Allocated and assigned in aweOS_init by default */
		UINT32 fastHeapASize; 							/**< Size of fast heap A in 32-bit words. Default 5000000. If specified without corresponding heap ptr also specified, aweOS_init will allocate a heap of this size */
		UINT32 fastHeapBSize; 							/**< Size of fast heap B in 32-bit words. Default 5000000. If specified without corresponding heap ptr also specified, aweOS_init will allocate a heap of this size */
		UINT32 slowHeapSize; 							/**< Size of slow heap in 32-bit words. Default 5000000. If specified without corresponding heap ptr also specified, aweOS_init will allocate a heap of this size */
		INT32(*cbAudioStart)(AWEOSInstance *pAWEOS); 	/**< Pointer to user created callback function for audio start commands. Default NULL */
		INT32(*cbAudioStop)(AWEOSInstance *pAWEOS); 	/**< Pointer to user created callback function for audio stop commands. Default NULL */
		UINT32 *pPacketBuffer; 							/**< Pointer to packet buffer. Only modify if not using built-in tuning interface aweOS_tuningSocketOpen. Default NULL */
		UINT32 *pReplyBuffer; 							/**< Pointer to reply packet buffer. Only modify if not using built-in tuning interface aweOS_tuningSocketOpen. Default NULL */
		UINT32 packetBufferSize; 						/**< Size of packet buffers. Default 264 32-bit words. Normally unchanged from default */
		UINT32 userVersion; 							/**< User specified version number. Default 1 */
		float coreSpeed; 								/**< Processor clock speed in Hz. Default 1 GHz */
		float profileSpeed; 							/**< Application profiling speed in Hz. Default 10 MHz. Normally unchanged from default */
		const char *pName; 								/**< Name of target. Max size 8 chars. Default "aweOS" */
		UINT32 numThreads; 								/**< Maximum number of supported sublayouts. Default 4. Max value is 31  */
		float sampleRate; 								/**< Sample rate of audio device. Default 48000.0 */
		UINT32 fundamentalBlockSize; 					/**< Fundamental block size of audio driver. Default 32 */
		UINT32 inChannels; 								/**< Number of input channels of audio device. Default 2 */
		UINT32 outChannels; 							/**< Number of output channels of audio device. Default 2 */
		INT32 instanceId; 								/**< ID number of instance. Default 0. Modify only in multi-instance applications */
	} AWEOSConfigParameters;

/** @}*/

/** 
* \defgroup AWECoreAPI AWECoreOS API
* @{
*/
	/**
	* @brief Return the PIDs of all internally spawned AWE Core OS threads. 
	* @details Get the PIDs of internal AWE Core OS pump, work, and socket threads. 
	* Provide a pointer to an AWEOSThreadPIDs struct, which is populated with available IDs when the API is called.
	* See the AWEOSThreadPIDs struct for details. The max size of the pumpThreadPIDs array will always be numThreads
	* If no layout is loaded, numPumpThreads and pumpThreadPIDs are zeroed. If the user does not open a tuning socket, socketThreadPID = 0.
	*
	* Note: Only internally spawned pump thread PIDs will be returned, NOT the main audio thread (created by user, calls aweOS_audioPumpAll()).
	* In low latency mode (layoutBS == fundamentalBS), the base layout audio pump is executed on the MAIN USER AUDIO THREAD.
	* As a result, this API will skip the base layout, and begin the PID returning at the second layout.
	*
	* @param [in] pAWEOS				AWE Core OS instance pointer
	* @param [in] threadPIDs			AWEOSThreadPIDs struct ptr which is populated with PIDs
	* @return							@ref E_SUCCESS, @ref E_NOT_CREATED, @ref E_SYSCALL
	*/
	UINT32 aweOS_getThreadPIDs(AWEOSInstance *pAWEOS, AWEOSThreadPIDs_t * threadPIDs);

	/**
	* @brief Populates an AWEOSConfigParameters structure with defaults
	* @details  Once it has been populated with defaults, the user can overwrite the members as needed for custom configuration. 
	*
	* If heap pointers are left as NULL, then the @ref aweOS_init function internally allocates heaps of the sizes specified by the heapSize parameters.
	* If a user wants specific control of heap memory locations, allocate the memory for the heaps before calling aweOS_init and overwrite the heapSize parameters with the allocated sizes.
	*
	* pPacketBuffer and pReplyBuffer should only be non-NULL if a user created tuning interface is being used in place of @ref aweOS_tuningSocketOpen.
	* @param [out] aweParams				the AWEOSConfigParameters struct
	* @return								 @ref E_SUCCESS, @ref E_EXCEPTION 
	*/
	INT32 aweOS_getParamDefaults(AWEOSConfigParameters *aweParams);

	/**
	* @brief Initialize the AWEOSInstance with the specified configuration parameters.
	* @details A user will define an AWEOSInstance pointer (handle) and pass a pointer to it (double pointer) to this function, along with the configured aweParams struct (see @ref AWEOSConfigParameters).
	* The AWEOSInstance will be allocated and then its members will be set with the values of the user's aweParams. 
	* This function must be called before using any other function that requires an AWEOSInstance as an argument. Failure to do so will lead to memory corruption and program crashes.
	* @param [out] pAWEOS						Double pointer to the AWEOSInstance variable. This will be the "handle" of the AWEOSInstance. 
	* @param [in] aweParams						The user's AWEOSConfigParameters structure 
	* @param [in] pModuleDescriptorTable		A ptr to the module table (see ModuleList.h) 
	* @param [in] moduleDescriptorTableSize		The size of the moduleDescriptorTable
	* @return							         @ref E_SUCCESS,  @ref E_HEAPS_NOT_INITIALIZED,  @ref E_HEAPS_ALREADY_INITIALIZED,  @ref E_PARAMETER_ERROR,  @ref E_CANNOT_CREATE_THREAD 
	*/
	INT32 aweOS_init(AWEOSInstance **pAWEOS, const AWEOSConfigParameters *aweParams, const void* pModuleDescriptorTable, UINT32 moduleDescriptorTableSize);

	/**
	* @brief Initialize and open an integrated TCP/IP tuning interface socket.
	* @details AWE Core OS has an optional integrated tuning interface that is enabled by calling this function after the AWEOSInstance has been initialized.
	* A TCP socket is opened on the specified port for the passed in instance.
	* For multiple AWEOSInstances on a single tuning interface, pass in the array of instances and the number of instances.
	* For an a single shared tuning interface, each AWEOSInstance's instanceID should be uniquely configured to 0, 16, 32, etc.
	* For multiple AWEOSInstances where each instance has it's own tuning interface, call this function for each instance with unique ports for each call.
	* For multiple sockets, both instance's should be configured with instanceID member set to 0. See the MultiInstance example for more information. 
	* @param [in] pAWEOS				AWE Core OS instance double pointer
	* @param [in] portNo				port to open the socket on. Must be even number between 15002 and 15098
	* @param [in] numInstances			number of AWEOSInstances that the opened socket will service. If using Multi-Instance single socket, then this will be the number of instances in the array. For single instances, always pass in 1.
	* @return							 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_INVALID_PORTNO,  @ref E_SOCKET_OPEN_FAIL,  @ref E_SCHEDULER_PERMISSION_DENIED
	*/
	INT32 aweOS_tuningSocketOpen(AWEOSInstance** pAWEOS, INT32 portNo, UINT32 numInstances);

	/**
	* @brief Close a running integrated TCP tuning socket.
	* @details Call this function to close an AWEOSInstance's integrated tuning socket.
	* Note: For a multi-instance tuning integrated socket, aweOS_tuningSocketClose only needs to be called on the first instance in the array.
	* For multiple sockets, aweOS_tuningSocketClose should be called for each instance with an open socket. 
	* @param [in] pAWEOS				AWE Core OS instance pointer
	*/
	void aweOS_tuningSocketClose(AWEOSInstance *pAWEOS);

	/**
	* @brief Enable logging of the tuning packets sent and received by the AWEOSInstance. Will create new uniquely named .log files
	*			in the specified directory. Logging tuning packets can be useful for debugging purposes. This will only log packets
	*			sent and received by the internal socket based tuning interface enabled with @ref aweOS_tuningSocketOpen.
	*			Tuning logging is disabled by default. 
	* @param [in] pAWEOS					AWE Core OS instance pointer
	* @param [in] path						Path to desired logging directory. Will use current directory if NULL or ""
	* @param [in] baseName					Base name of logging file. Will use 'awe_tuning' if NULL or ""
	* @param [in] verbosity					Verbosity level of tuning log. One of @ref TUNING_LOG_ERROR, @ref TUNING_LOG_INFO, @ref TUNING_LOG_DATA
	* @return								 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_CANTCREATE,  @ref E_CANTOPEN 
	*/
	INT32 aweOS_tuningLoggingEnable(AWEOSInstance *pAWEOS, char* path, char* baseName, UINT32 verbosity);

	/**
	* @brief Executes packet commands from an in-memory array. AWE Designer can generate AWB arrays directly from a layout, which can then be compiled directly into the application. 
	* See Tools->Generate Target Files in Designer.
	* @param[in] pAWEOS 		AWE Core OS instance pointer
	* @param[in] pArray  		Array of commands
	* @param[in] arraySize 		Size of command array 
	* @param[out] pErrorOffset	Report failing word index
	* @return 					 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_ZERO_PACKET_BUFFER_LENGTH,  @ref E_NULL_PACKET_BUFFER_POINTER,
	*  @ref E_NULL_REPLY_BUFFER_POINTER,  @ref E_EXCEPTION,  @ref E_UNEXPECTED_EOF
	*/
	INT32 aweOS_loadAWBFromArray(AWEOSInstance *pAWEOS, const UINT32 *pArray, UINT32 arraySize, UINT32 * pErrorOffset);


	/**
	* @brief Executes packet commands from an AWB file on the filesystem. AWB files can be generated for any layout from AWE Designer using Tools->Generate Target Files.
	* @param[in] pAWEOS 		AWE Core OS instance pointer
	* @param[in] binaryFile  	full filepath to AWB file on fileystem
	* @param[out] pErrorOffset	Report failing word index
	* @return 					 @ref E_SUCCESS,  @ref E_CANTOPEN,  @ref E_NOSUCHFILE,  @ref E_NOT_CREATED,  @ref E_ZERO_PACKET_BUFFER_LENGTH,  @ref E_NULL_PACKET_BUFFER_POINTER,
	*  @ref E_NULL_REPLY_BUFFER_POINTER,  @ref E_EXCEPTION,  @ref E_UNEXPECTED_EOF
	*/
	INT32 aweOS_loadAWBFile(AWEOSInstance *pAWEOS, const char *binaryFile, UINT32 *pErrorOffset);

	/**
	* @brief Returns the number of input and output channels in the loaded layout.
	* @param [in] pAWEOS				AWE Core OS instance pointer
	* @param [out] inCount				number of input channels
	* @param [out] outCount				number of output channles
	* @return							 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS
	*/
	INT32 aweOS_layoutGetChannelCount(const AWEOSInstance *pAWEOS, UINT32 *inCount, UINT32 *outCount);
	
	/**
	* @brief Returns the block size of the loaded layout. 
	* Input and output blockSize are the same for all layouts.
	* @param [in] pAWEOS				AWE Core OS instance pointer
	* @param [out] blockSize			block size of layout
	* @return							 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS
	*/
	INT32 aweOS_layoutGetBlockSize(const AWEOSInstance *pAWEOS, UINT32 *blockSize);

	/**
	* @brief Returns the sample rate of the loaded layout. 
	* Input and output sample rates are the same for all layouts.
	* @param [in] pAWEOS				AWE Core OS instance pointer
	* @param [out] sampleRate			sample rate of layout
	* @return							 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS
	*/	
	INT32 aweOS_layoutGetSampleRate(const AWEOSInstance *pAWEOS, FLOAT32 *sampleRate);
	
	/**
	* @brief Pump one fundamental block size of audio through the loaded layout and all of its sublayouts
	* @details This function will coordinate the audio processing threads to process audio through the loaded layouts and sublayouts as needed.
	* The user calls this function in their audio callback, which should be set to the highest priority possible on the system, after calling @ref aweOS_audioImportSamples and @ref aweOS_audioExportSamples.
	* Each sublayout (including the base layout) gets its own thread where the audio will be pumped. The priority level of each sublayout's pump thread will decrement by one from the audio callback's priority level.
	* For example, if the audio callback has priority 99, and the loaded layout has 2 sublayouts, 1 with a clockdivider of 1 and the other with a clockdivider of 16, thread priority levels will be: (audiocallback: 99, pumpthread0: 98, pumpthread1: 97).
	* AWE Core OS names these threads if given permission, and the names/priorities can be seen with top/htop in Linux.
	* To ensure a close to real-time priority system, the priority level of the audio callback that calls this function must be greater than (AWEOSConfigParameters.numThreads + 2).
	* If this function is not called at a high enough priority to set the decreasing priorities, all pumpthreads are set to the current priority, and  @ref E_PRIORITY_NOT_HI_ENOUGH is returned. Users can ignore this error at their own risk.
	* If this function isn't called at any RT priority level, then  @ref E_CALLBACK_NOT_REALTIME is returned.
	* Note: Linux requires elevated privileges to change thread priority levels (i.e., application should be run as root). If the application does not have permission to set priorities, then  @ref E_PRIORITY_NOT_HI_ENOUGH is returned.
	* @param [in] pAWEOS				AWE Core OS instance pointer
	* @return							 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS,  @ref E_SCHEDULER_PERMISSION_DENIED,  @ref E_CANNOT_CREATE_THREAD,  @ref E_PRIORITY_NOT_HI_ENOUGH,  @ref E_CALLBACK_NOT_REALTIME
	*/
	INT32 aweOS_audioPumpAll(AWEOSInstance *pAWEOS);


	/**
 	* @brief Import samples from an audio buffer to a specific channel of the AWEOSInstance's input pin. The value of the fundamentalBlockSize member of the AWEOSInstance determines the number of samples that are imported with each call.
	Call this function once for each audio channel available from the input audio device.
	Channel matching between hardware capabilities and layout will be done automatically. For example, if the audio device has only 1 audio input channel, 
	and if a layout with 3 input channels is loaded, the second and third channels of the layout input will be all zeros. Inversely, if there are more
	input audio device channels than the layout expects, then they are ignored.
	
	The value of inStride defines the number of samples to skip between each read, and should be the number of interleaved channels in the user's audio buffer being read from. 
	A typical usage for a single source, interleaved stereo input of 32-bit samples could look like this:
	@code
		aweOS_audioImportSamples(pAWEOS, &inSamples[0], 2, 0, Sample32bit);
		aweOS_audioImportSamples(pAWEOS, &inSamples[1], 2, 1, Sample32bit);
	@endcode
	* @param [in] pAWEOS						AWE Core OS instance pointer
	* @param [in] inSamples						buffer to read input samples from
	* @param [in] inStride						input buffer stride
	* @param [in] channel						channel of instance pin to write to
	* @param [in] inType						type of input data
	* @return									 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_PARAMETER_ERROR,  @ref E_NO_LAYOUTS,  @ref E_AUDIO_NOT_STARTED
	*/
	INT32 aweOS_audioImportSamples(AWEOSInstance *pAWEOS, void *inSamples, INT32 inStride, INT32 channel, SampleType inType);

	/**
 * @brief Export samples to a user buffer from a specific channel of the AWEOSInstance's output pin. The value of the fundamentalBlockSize member of the AWEOSInstance determines the number of samples that are exported with each call.
	Call this function once for each audio output channel available on audio device. 
	Like @ref aweOS_audioImportSamples, this will do channel matching automatically. For example, if the audio device has 3 audio output channels, 
	and if a layout with 1 output channel is loaded, the second and third channels of the exported output will be all zeros. Inversely, if
	the layout has more output channels than the audio device supports, they are ignored.

	The value of outStride defines the number of samples to skip between each write. It should be equal to the number of interleaved channels in the outSamples buffer being written to. 
	A typical usage for exporting to an interleaved 3 channel output could look like:
	@code
		aweOS_audioExportSamples(pAWEOS, &outSamples[0], 3, 0, Sample32bit);
		aweOS_audioExportSamples(pAWEOS, &outSamples[1], 3, 1, Sample32bit);
		aweOS_audioExportSamples(pAWEOS, &outSamples[2], 3, 2, Sample32bit);
	@endcode
	
	* @param [in] pAWEOS						AWE Core OS instance pointer
	* @param [out] outSamples					buffer to write output audio to
	* @param [in] outStride						output buffer stride
	* @param [in] channel						channel of instance pin to read from
	* @param [in] outType						type of output data
	* @return									 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_PARAMETER_ERROR,  @ref E_NO_LAYOUTS,  @ref E_AUDIO_NOT_STARTED 
	*/
	INT32 aweOS_audioExportSamples(AWEOSInstance *pAWEOS, void *outSamples, INT32 outStride, INT32 channel, SampleType outType);

	/**
	* @brief Determines if a layout is loaded and valid. Use before processing audio in audio callback to avoid pumping through a dead layout.
	*	In some applications, the user may want to pass input audio straight to the output device if @ref aweOS_audioIsStarted is TRUE but this function returns FALSE. 
	*	This design can be useful for debugging audio drivers. 
	* @param [in] pAWEOS				    AWE Core OS instance pointer
	* @return								1 if layout is valid, 0 if invalid, or  @ref E_NOT_CREATED					
	*/
	INT32 aweOS_layoutIsValid(const AWEOSInstance *pAWEOS);

	/**
	* @brief Check if this instance has received an Audio Start command. Audio Start can be received over the tuning interface, and it is also usually the last command in an AWB file.
	* 		Use this before pumping audio to avoid processing before it has been requested. This command will begin returning 0 once an Audio Stop command is received. 
	* @param [in] pAWEOS					AWE Core OS instance pointer
	* @return								1 if started, 0 if not started,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS
	*/
	INT32 aweOS_audioIsStarted(const AWEOSInstance *pAWEOS);

	/**
	* @brief Convert an error code (INT32) to its corresponding error string.
	* @param [in] errorCode					error code that you wish to convert
	* @return								error as string, or failures "invalid error code, not a negative number", "not a valid error. error code out of range" 
	*/
	const char* aweOS_errorToString(INT32 errorCode);

	/**
	* @brief Process the packet buffer in the AWEOSInstance. 
	* @details This function should never be called if an AWE Core OS integrated tuning socket is open (see @ref aweOS_tuningSocketOpen). This function is used only by user created tuning interfaces to process received tuning commands. 
	* 			See the @ref TuningProtocol document for details.
	* @param[in] pAWEOS				AWE Core OS instance pointer
	* @return						 @ref E_SUCCESS,  @ref E_NOT_CREATED
	*/
	INT32 aweOS_tuningPacketProcess(AWEOSInstance *pAWEOS);

	/**
	* @brief Set a scalar or array value(s) of a module parameter by handle. 
	* Module handles and parameter sizes can be generated for any layout using Tools->Generate Target Files in AWE Designer. Select the ControlInterface.h checkbox to generate control files. 
	* @param [in] pAWEOS						AWE Core OS instance pointer
	* @param [in] handle						packed object handle
	* @param [in] value							value(s) to set
	* @param [in] arrayOffset					array offset if array. 0 if scalar.
	* @param [in] length						number of elements to set if array. 1 if scalar.
	* @return									 @ref E_SUCCESS,  @ref E_ARGUMENT_ERROR,  @ref E_BAD_MEMBER_INDEX,  @ref E_CLASS_NOT_SUPPORTED,  @ref E_LINKEDLIST_CORRUPT,  @ref E_NO_MORE_OBJECTS,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS
	*/
    INT32 aweOS_ctrlSetValue(const AWEOSInstance *pAWEOS, UINT32 handle, void *value, INT32 arrayOffset, UINT32 length);
	
	/**
	* @brief Get a scalar or array value(s) of a module parameter by handle. 
	* Module handles and parameter sizes can be generated for any layout using Tools->Generate Target Files in AWE Designer. Select the ControlInterface.h checkbox to generate control files. 
	* @param [in] pAWEOS						AWE Core OS instance pointer
	* @param [in] handle						packed object handle
	* @param [out] value						retrieved value(s) 
	* @param [in] arrayOffset					array offset if array. 0 if scalar
	* @param [in] length						number of elements to retrieve if array. 1 if scalar
	* @return									 @ref E_SUCCESS,  @ref E_ARGUMENT_ERROR,  @ref E_BAD_MEMBER_INDEX,  @ref E_CLASS_NOT_SUPPORTED,  @ref E_LINKEDLIST_CORRUPT,  @ref E_NO_MORE_OBJECTS,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS
	*/
	INT32 aweOS_ctrlGetValue(const AWEOSInstance *pAWEOS, UINT32 handle, void *value, INT32 arrayOffset, UINT32 length);

	/**
	* @brief Set the status of a module. Valid status values are:
	* @details 	0=Active
	*
	* 1=Bypass
	*
	* 2=Mute
	*
	* 3=Inactive
	*
	* Module handles sizes can be generated for any layout using Tools->Generate Target Files in AWE Designer. Select the ControlInterface.h checkbox to generate control files. 
	* @param [in] pAWEOS						AWE Core OS instance pointer
	* @param [in] handle						packed object handle
	* @param [in] status						status to set
	* @return									 @ref E_SUCCESS,  @ref E_ARGUMENT_ERROR,  @ref E_LINKEDLIST_CORRUPT,  @ref E_NO_MORE_OBJECTS,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS
	*/
	INT32 aweOS_ctrlSetStatus(const AWEOSInstance *pAWEOS, UINT32 handle, UINT32 *status);

	/**
	* @brief Get the status of a module.
	* Module handles sizes can be generated for any layout using Tools->Generate Target Files in AWE Designer. Select the ControlInterface.h checkbox to generate control files. 
	* @param [in] pAWEOS						AWE Core OS instance pointer
	* @param [in] handle						packed object handle
	* @param [out] status						returned status
	* @return									 @ref E_SUCCESS,  @ref E_ARGUMENT_ERROR,  @ref E_LINKEDLIST_CORRUPT,  @ref E_NO_MORE_OBJECTS,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS,  @ref E_NOT_MODULE
	*/
	INT32 aweOS_ctrlGetStatus(const AWEOSInstance *pAWEOS, UINT32 handle, UINT32 *status);

	/**
	* @brief Set a scalar or array value(s) of a module variable by handle with mask. A mask allows you to only call module's set function
	*      for a single variable, or skip the set function all together. For advanced users.
	* @param [in] pAWEOS					AWE Core OS instance pointer
	* @param [in] handle					packed object handle
	* @param [in] value						value(s) to set
	* @param [in] arrayOffset				array offset if array. 0 if scalar
	* @param [in] length					number of elements if array. 1 if scalar
	* @param [in] mask						mask to use - 0 to not call set function.
	* @return								 @ref E_SUCCESS,  @ref E_ARGUMENT_ERROR,  @ref E_BAD_MEMBER_INDEX,  @ref E_CLASS_NOT_SUPPORTED,  @ref E_OBJECT_ID_NOT_FOUND,  @ref E_NOT_MODULE,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS
	*/
	INT32 aweOS_ctrlSetValueMask(const AWEOSInstance *pAWEOS, UINT32 handle, void *value, INT32 arrayOffset, UINT32 length, UINT32 mask);

	/**
	* @brief Get a scalar or array value(s) of a module variable by handle with mask. A mask allows you to only call module's get function
	*      for a single variable, or skip the get function all together. For advanced users.
	* @param [in] pAWEOS					AWE Core OS instance pointer
	* @param [in] handle					packed object handle
	* @param [out] value					value(s) to get
	* @param [in] arrayOffset				array offset if array. 0 if scalar.
	* @param [in] length					number of elements if array. 1 if scalar
	* @param [in] mask						mask to use - 0 to not call get function
	* @return								 @ref E_SUCCESS,  @ref E_ARGUMENT_ERROR,  @ref E_OBJECT_ID_NOT_FOUND,  @ref E_CLASS_NOT_SUPPORTED,  @ref E_NOT_MODULE,  @ref E_BAD_MEMBER_INDEX
	*/
	INT32 aweOS_ctrlGetValueMask(const AWEOSInstance *pAWEOS, UINT32 handle, void *value, INT32 arrayOffset, UINT32 length, UINT32 mask);

	/**
	* @brief Get a module's object class from its handle.
	* @param [in] pAWEOS					instance pointer
	* @param [in] handle					handle of object to find
	* @param [out] pClassID					pointer to found object class
	* @return								 @ref E_SUCCESS,  @ref E_NO_MORE_OBJECTS,  @ref E_LINKEDLIST_CORRUPT,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS
	*/
	INT32 aweOS_ctrlGetModuleClass(const AWEOSInstance *pAWEOS, UINT32 handle, UINT32 *pClassID);

	/**
	* @brief Destroys the AWEOSInstance and closes all associated threads. Frees all internal memory and sets the AWEOSInstance to NULL.
	* @param [in] pAWEOS				pointer to pointer of AWE Core OS instance to destroy
	* @return							 @ref E_SUCCESS,  @ref E_NOT_CREATED
	*/
	INT32 aweOS_destroy(AWEOSInstance **pAWEOS);

	/**
	* @brief Enable or disable the profiling ability of the AWE Core OS Instance. Both top and module level profiling enabled by default by @ref aweOS_init. 
	* Use this function to selectively enable or disable per pump profiling during runtime. 
	* Disabling profiling saves a small amount of cycles per pump. Do not call this API from the audio start/stop callbacks. 
	* User can also enable or disable independently module level profiling and top level profiling.
	* @param [in] pAWEOS					AWE Core OS instance pointer
	* @param [in] status					0 to disable both, 1 to enable both, 2 to enable module level only and 3 to enable top level only
	* @return								@ref E_SUCCESS, @ref E_NOT_OBJECT, @ref E_NOT_CREATED
	*/
	INT32 aweOS_setProfilingStatus(AWEOSInstance *pAWEOS, UINT32 status);
   
    /**
    * @brief Get the average cycles of a running layout, in units of cycles at profileSpeed. Returns cycles in 24.8 format, 
	* so shift right by 8 bits for integer value. To get CPU cycles, multiply by target cpuSpeed / profileSpeed. 
    * @param [in] pAWEOS					AWE Core OS instance pointer
    * @param [in] idx						Layout index (typically 0, except in advanced use cases)
    * @param [out] averageCycles         	Pointer to average cycles
    * @return								 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_NO_LAYOUTS,  @ref E_PARAMETER_ERROR
    */
    INT32 aweOS_getAverageLayoutCycles(AWEOSInstance *pAWEOS, UINT32 idx, UINT32 * averageCycles);

    /**
    * @brief Get the version information of the AWE Core OS library. Can be used without initializing an AWEOSInstance.
	*  AWE Core OS versions are of form <tuningVer>.<majorAPIVer>.<minorAPIVer>.<processorVer>. All values are 
	*  integers execpt for majorAPIVer, which is a single, capital letter.  
	* 
    * @param [out] versionInfo				Pointer to a version info data structure
	*/
	void aweOS_getVersion(AWEOSVersionInfo_t * versionInfo);

	/**
	* @brief Enables recording of all input and output audio of AWEOSInstance. Will create unique input and output .wav files
	*			in the specified directory. Recording input and output audio can be useful for debugging purposes. 
	*			Audio recording happens in a background thread so as not to interrupt real time audio processing. 
    *           If the system is heavily loaded, it is possible that overruns can occur while writing the audio to files. 
    *           If overruns do occur on the system, increasing the size of bufferLength and using the 'Sample16bit' sampleType
    *           can help avoid missed audio samples in the recorded files. 
	*			Audio recording is disabled by default at @ref aweOS_init time. Call this function after aweOS_init to enable recording on the next Audio Start command.
    *           For greater control of audio recording capabilities, see the Wave File Source and Sink modules in Designer.
	*
	*			Recording audio during real-time processing 
	* @param [in] pAWEOS					AWE Core OS instance pointer
	* @param [in] path						Path to desired logging directory. Will use the directory application is called from if NULL or ""
	* @param [in] baseName					Base name of audio files. Will use 'awe_audio' if NULL or ""
	* @param [in] bufferLength				Length in milliseconds of audio FIFO used to buffer audio data for writing to file. Increase to handle large system I/O costs and avoid xruns
	* @param [in] sampleType				Data type of recorded audio. Supports 'Sample32bit' and 'Sample16bit' only
	* @return								 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_PARAMETER_ERROR
	*/
	INT32 aweOS_audioRecordingEnable(AWEOSInstance * pAWEOS, char* path, char* baseName, UINT32 bufferLength, SampleType sampleType);

	/**
	* @brief Register a callback function for audio recording event notificiations. Events include errors (file create or I/O) or xruns.
	*			Mask controls which type of notifications to enable for callback. First bit of mask enables error notifications, second bit enables xrun notifications.
	*			An @ref AWEOSAudioRecordNotification_t structure is passed as the only argument to the callback function.
	* @param [in] pAWEOS							AWE Core OS instance pointer
	* @param [in] recordNotificationCallback		Function pointer to register for error/info notification during recording
	* @param [in] recordNotificationMask			Bit mask to control notification types
	* @return										 @ref E_SUCCESS,  @ref E_NOT_CREATED,  @ref E_PARAMETER_ERROR
	*/
	INT32 aweOS_audioRecordingRegisterNotificationCallback(AWEOSInstance * pAWEOS, recordNotificationCallbackFunction recordNotificationCallback, UINT32 recordNotificationMask);

	/**
	* @brief Disable recording of input and output audio of AWEOSInstance. Can be called during runtime while audio recording is ongoing
	*			to halt recording. To record audio again on next Audio Start command, must call @ref aweOS_audioRecordingEnable again.
	* @param [in] pAWEOS						AWE Core OS instance pointer
	* @return									 @ref E_SUCCESS,  @ref E_NOT_CREATED
	*/
	INT32 aweOS_audioRecordingDisable(AWEOSInstance *pAWEOS);

	/**
	 * @brief Open a .wav file and populate the user arguments with the header information in the file. Returns the pointer to the opened file in the fp argument.
	 * 			No audio data is read in this function call. Use @ref aweOS_wavFileRead to access the audio data after the file is opened. @ref aweOS_wavFileClose
	 * 			can be used to close the file once fully read.
	 * @param [in]  file						full path to name of the .wav file
	 * @param [out] sampleRate					sample rate value extracted from header
	 * @param [out] numChannels					number of audio channels extracted from header
	 * @param [out] sampleSize					size of each audio sample, in bytes
	 * @param [out] numSamples					total number of audio samples in file
	 * @param [out] fp							pointer to opened FILE
	 * @return 									@ref E_SUCCESS, @ref E_IOERROR, @ref E_ILLEGAL_FILE_ATTRIBUTE,  @ref E_CANTOPEN
	 */
	INT32 aweOS_wavFileOpen(const char *file, FLOAT32 *sampleRate, UINT32 *numChannels, UINT32 *sampleSize, UINT32 *numSamples, FILE ** fp);

	/**
	 * @brief Create a .wav file and populate the header with the passed in arguments. Returns the pointer to the created file in the fp argument.
	 * 			No audio data is written in this function call. Use @ref aweOS_wavFileWrite to write the audio data after the file is created. @ref aweOS_wavFileClose
	 * 			can be used to close the file once fully written.
	 * @param [in]  file						full path to name of the .wav file
	 * @param [in]  sampleRate					sample rate
	 * @param [in]  numChannels					number of audio channels in data to be written
	 * @param [in]  sampleSize					size of each audio sample, in bytes
	 * @param [out] fp							pointer to created FILE
	 * @return 									 @ref E_SUCCESS,  @ref E_PARAMETER_ERROR,  @ref E_IOERROR,  @ref E_CANTCREATE
	 */
	INT32 aweOS_wavFileCreate(const char *file, FLOAT32 sampleRate, UINT32 numChannels, UINT32 sampleSize, FILE ** fp);

	/**
	 * @brief Write audio data to .wav file created using @ref aweOS_wavFileCreate. 
	 * @param [in] fp							pointer to file to write data to
	 * @param [in] samples						pointer to buffer of samples to write to file
	 * @param [in] numSamples					total number of samples to write
	 * @param [in] sampleSize					size of each audio sample, in bytes
	 * @return 									number of samples written,  @ref E_BADFILE,  @ref E_IOERROR
	 */
	INT32 aweOS_wavFileWrite(FILE * fp, void * samples, UINT32 numSamples, UINT32 sampleSize);

	/**
	 * @brief Read audio data from .wav file opened using @ref aweOS_wavFileOpen. 
	 * @param [in]  fp							pointer to file to read data from
	 * @param [out] samples						pointer to buffer to populate with samples from file
	 * @param [in]  numSamples					total number of samples to read
	 * @param [in]  sampleSize					size of each audio sample, in bytes
	 * @return 									number of samples read,  @ref E_BADFILE,  @ref E_IOERROR
	 */
	INT32 aweOS_wavFileRead(FILE * fp, void * samples, UINT32 numSamples, UINT32 sampleSize);

		/**
	 * @brief Close the.wav file opened using @ref aweOS_wavFileOpen or @ref aweOS_wavFileCreate. 
	 * @param [in]  fp							pointer to file to read data from
	 * @return 									 @ref E_SUCCESS,  @ref E_NO_OPEN_FILE
	 */
	INT32 aweOS_wavFileClose(FILE * fp);

	/**
	 * @brief Profiling related setup function only intended for systems with multiple AWEOS Instances
	 *        in a single application. Call this function at startup after aweOS_init is done in the sequence.
	 * @param [in] pInstances			Array of AWEOS Instance pointers. Array must exist as long as the 
										system is active.
	 * @param [in] numInstances			Number of AWEOS Instances in pInstances array.
	 * @return 							@ref E_SUCCESS,  @ref E_PARAMETER_ERROR
	*/ 
    INT32 aweOS_setInstancesInfo(AWEOSInstance **pInstances, INT32 numInstances);

/** @}*/

#ifdef __cplusplus
}
#endif

#endif // AWECOREOS_H_