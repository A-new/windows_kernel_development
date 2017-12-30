#include "NPminifilter.h"
//  Global variables

ULONG gTraceFlags = 0;

PFLT_FILTER gFilterHandle;
PFLT_PORT 	gServerPort;
PFLT_PORT 	gClientPort;

NPMINI_COMMAND gCommand = ENUM_BLOCK;

//	ʹ����������ƺ��ǿ���������ġ�����
#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(gTraceFlags,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((void)0))

/*
	����˵�����ú���ʵ���˽� UNICODE �ַ���ת��Ϊ ȫ�Ǵ�д�� char ���飬�Ա��ڲ��ҡ�NOTEPAD.EXE����
	������
	����ֵ��TRUE OR FALSE
*/
BOOLEAN NPUnicodeStringToChar(PUNICODE_STRING UniName, char Name[])
{
    ANSI_STRING	AnsiName;			//	ANSI_STRING �е� Buffer �� PCHAR ���ͣ���UNICODE_STRING �е� Buffer �� PWSTR ����
    NTSTATUS	ntstatus;
    char*		nameptr;			
    
   DbgPrint("NPminifilter!NPUnicodeStringToChar: Entered\n"); 
    
    __try {	    		   		    		
			ntstatus = RtlUnicodeStringToAnsiString(&AnsiName, UniName, TRUE);		
			
			//	260 ��FileName �������Ĵ�С				
			if (AnsiName.Length < 260) {
				
				nameptr = (PCHAR)AnsiName.Buffer;
				//Convert into upper case and copy to buffer
				
				strcpy(Name, _strupr(nameptr));						    	    
			//	DbgPrint("NPUnicodeStringToChar : %s\n", Name);	
			}		  	
			RtlFreeAnsiString(&AnsiName);		 
	} 
	__except(EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint("NPUnicodeStringToChar EXCEPTION_EXECUTE_HANDLER\n");	
		return FALSE;
	}
    return TRUE;
}      
/*
	���ڱ����򣬸ú�����ûʵ�����壬��������Ϊʾ����
	
	�ú������ڵ�Ŀ�ģ���Ҫ���������������������ĸ�����Ҫ�󶨣��ĸ�����Ҫ�󶨡�
	�����¾������Ǵ����ûص�������

*/
NTSTATUS
NPInstanceSetup (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_SETUP_FLAGS Flags,
    __in DEVICE_TYPE VolumeDeviceType,
    __in FLT_FILESYSTEM_TYPE VolumeFilesystemType
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );

    PAGED_CODE();
    
    
		 DbgPrint("NPminifilter!NPInstanceSetup: Entered\n");
	
    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("NPminifilter!NPInstanceSetup: Entered\n") );

    return STATUS_SUCCESS;
}

/*
	���ڱ�������˵���ûص�����û���κ����壬���в�û����ʲô����������Ϊʾ����
	
	InstanceQueryTeardownΪ����ʵ�����ٺ�����ֻ����ֻ���ֹ���󶨵�ʱ�򱻴�����

*/

NTSTATUS
NPInstanceQueryTeardown (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

		 DbgPrint("NPminifilter!NPInstanceQueryTeardown: Entered\n");
    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("NPminifilter!NPInstanceQueryTeardown: Entered\n") );

    return STATUS_SUCCESS;
}

/*
		�Ա�����û��ʲôʵ�����塣
		
		InstanceTeardownStartΪʵ����󶨻ص�������������ʱ�Ѿ���������󶨡�


*/
VOID
NPInstanceTeardownStart (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();
    
    DbgPrint("NPminifilter!NPInstanceTeardownStart: Entered\n");

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("NPminifilter!NPInstanceTeardownStart: Entered\n") );
}

/*
	�Ա�����û��ʲôʵ������
	
	InstanceTeardownCompleteΪʵ����󶨵���ɺ�������ȷ��ʱ���ý���󶨺����ɺ�����

*/
VOID
NPInstanceTeardownComplete (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();
		DbgPrint("NPminifilter!NPInstanceTeardownComplete: Entered\n");
    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("NPminifilter!NPInstanceTeardownComplete: Entered\n") );
}


/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    __in PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS status;
    PSECURITY_DESCRIPTOR sd;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING uniString;		//for communication port name
    
    UNREFERENCED_PARAMETER( RegistryPath );

		 DbgPrint("NPminifilter!DriverEntry: Entered\n");  

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("NPminifilter!DriverEntry: Entered\n") );

    //
    //  Register with FltMgr to tell it our callback routines
    //

    status = FltRegisterFilter( DriverObject,	//	����������������
                                &FilterRegistration,//	iһ������ע��Ľṹ����NPminifilter.h���ж���
                                &gFilterHandle );	//	����ע��ɹ���Ĺ��������

    ASSERT( NT_SUCCESS( status ) );

    if (NT_SUCCESS( status )) {

        //
        //  Start filtering i/o
        //

        status = FltStartFiltering( gFilterHandle );	//	���������֮ǰ�������ǲ������õġ�����������˿��������á�

        if (!NT_SUCCESS( status )) {

            FltUnregisterFilter( gFilterHandle );
        }
    }
    //
		//Communication Port
		//
		
		//	FltBuildDefaultSecurityDescriptor �� FLT_RORT_ALL_ACCESS Ȩ��������һ����ȫ�Ե�������
    status  = FltBuildDefaultSecurityDescriptor( &sd, FLT_PORT_ALL_ACCESS );
    
    if (!NT_SUCCESS( status )) {
       	goto final;
    }
                  
    RtlInitUnicodeString( &uniString, MINISPY_PORT_NAME );//	��ʼ��ͨ�Ŷ˿���
    
		// InitializeObjectAttributes ����ʼ���������ԣ�OBJECT_ATTRIBUTES��
    InitializeObjectAttributes( &oa,
                                &uniString,
                                OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                                NULL,
                                sd );
		
		//	����ͨ�Ŷ˿�
    status = FltCreateCommunicationPort( gFilterHandle,
                                         &gServerPort,
                                         &oa,
                                         NULL,
                                         NPMiniConnect,
                                         NPMiniDisconnect,
                                         NPMiniMessage,
                                         1 );

    FltFreeSecurityDescriptor( sd );

    if (!NT_SUCCESS( status )) {
        goto final;
    }            

final :
    
    if (!NT_SUCCESS( status ) ) {

         if (NULL != gServerPort) {
             FltCloseCommunicationPort( gServerPort );
         }

         if (NULL != gFilterHandle) {
             FltUnregisterFilter( gFilterHandle );
         }       
    }
    
    DbgPrint("driver entry end!\n");  
     
    return status;
}

/*
		ж�ػص��������ر�ͨ�Ŷ˿ڣ�ȡ��ע�ᡣ
*/
NTSTATUS
NPUnload (
    __in FLT_FILTER_UNLOAD_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

		DbgPrint("NPminifilter!NPUnload: Entered\n");  
    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("NPminifilter!NPUnload: Entered\n") );

		FltCloseCommunicationPort( gServerPort );
		
    FltUnregisterFilter( gFilterHandle );

    return STATUS_SUCCESS;
}


/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/

/*
	����Ԥ�����ص�������
		
			1��ͨ�� FltGetFileNameInformation ��������ļ�����Ϣ
			2������ȡ�ɹ�����ͨ�� FltParseFileNameInformation ���������ļ���
			3���������ɹ�����ͨ�� NPUnicodeStringToChar �������ļ���ת���ɴ�д�ַ���
			4����ת���ɹ�����ʹ�� strstr ƥ�� NOTEPADE.EXE 
			5��������ֵ�ǿգ�������ǣ���ô���ش��� 

*/
FLT_PREOP_CALLBACK_STATUS
NPPreCreate (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    )
{
		//
		//	��������������ȡ�ļ���
		//			
		char FileName[260] = "X:";
		
		NTSTATUS status;
		PFLT_FILE_NAME_INFORMATION nameInfo;

		//
		//	δʹ�ò������ú��ڸ�ʹ֮���������뾯��
		//
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

		//
		//	����ҳ����
		//
    PAGED_CODE();	        

		__try {	                             								
		    status = FltGetFileNameInformation( Data,
		                                        FLT_FILE_NAME_NORMALIZED |
		                                            FLT_FILE_NAME_QUERY_DEFAULT,
		                                        &nameInfo );
		    if (NT_SUCCESS( status )) {
		    		//�P����ɹ���������ļ�����Ϣ��Ȼ��Ƚ������Ƿ���NOTEPAD.EXE
		    	//	 DbgPrint("NPPreCreate��FilGetFileNameInformation success\n");
		    		if (gCommand == ENUM_BLOCK) {
		    			
						    FltParseFileNameInformation( nameInfo );
								if (NPUnicodeStringToChar(&nameInfo->Name, FileName)) {		
								//	DbgPrint("NPPreCreate:FilGetFileNameInformation success %s",FileName);
								//
								//	strstr() ��������һ���ַ�������һ���ַ����еĵ�һ�γ��֡�
								//	�ҵ����������ַ�������ú������ص�һ��ƥ����ַ����ĵ�ַ�����δ�ҵ����������ַ������򷵻�NULL��
								//	����ԭ�ͣ�extern char *strstr(char *str1, const char *str2);
								//	str1: ������Ŀ�꣬str2: Ҫ���Ҷ���
								//	����ֵ����str2��str1���Ӵ�������ȷ��str2��str1�ĵ�һ�γ��ֵ�λ�ã������ش�str1��str2��λ�õĵ�ַ��
								//	���str2����str1���Ӵ����򷵻�NULL��
								//
								//	�Լ����˸���ֹ EXPLORER.EXE	����
								//
								    if (strstr(FileName, "NOTEPAD.EXE") != NULL || strstr(FileName,"EXPLORER.EXE") != NULL) {

												Data->IoStatus.Status = STATUS_ACCESS_DENIED;
												Data->IoStatus.Information = 0;
												FltReleaseFileNameInformation( nameInfo );
												return FLT_PREOP_COMPLETE;						    		
								    }																			
								}						 
						}   			    
				    //release resource
				    FltReleaseFileNameInformation( nameInfo );        
		    }                                    
		}
		__except(EXCEPTION_EXECUTE_HANDLER) {
		    DbgPrint("NPPreCreate EXCEPTION_EXECUTE_HANDLER\n");				
		}

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

/*

	���ɺ�����ص����Ա�����û��ʲôʵ�����壬����ʾ��

*/

FLT_POSTOP_CALLBACK_STATUS
NPPostCreate (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
			
		
    FLT_POSTOP_CALLBACK_STATUS returnStatus = FLT_POSTOP_FINISHED_PROCESSING;
    PFLT_FILE_NAME_INFORMATION nameInfo;
    NTSTATUS status;

    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );
		
//		DbgPrint("NPPostCreate enter\n");
		
    //
    //  If this create was failing anyway, don't bother scanning now.
    //

    if (!NT_SUCCESS( Data->IoStatus.Status ) ||
        (STATUS_REPARSE == Data->IoStatus.Status)) {

        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    //
    //  Check if we are interested in this file.
    //

    status = FltGetFileNameInformation( Data,
                                        FLT_FILE_NAME_NORMALIZED |
                                            FLT_FILE_NAME_QUERY_DEFAULT,
                                        &nameInfo );

    if (!NT_SUCCESS( status )) {

        return FLT_POSTOP_FINISHED_PROCESSING;
    }
    		
    return returnStatus;
}

//
//	���û�̬���ں�̬��������ʱ����õ��ĺ���
//
NTSTATUS
NPMiniConnect(
    __in PFLT_PORT ClientPort,
    __in PVOID ServerPortCookie,
    __in_bcount(SizeOfContext) PVOID ConnectionContext,
    __in ULONG SizeOfContext,
    __deref_out_opt PVOID *ConnectionCookie
    )
{
		DbgPrint("[mini-filter] NPMiniConnect\n");
    PAGED_CODE();

    UNREFERENCED_PARAMETER( ServerPortCookie );
    UNREFERENCED_PARAMETER( ConnectionContext );
    UNREFERENCED_PARAMETER( SizeOfContext);
    UNREFERENCED_PARAMETER( ConnectionCookie );

    ASSERT( gClientPort == NULL );
    gClientPort = ClientPort;
    return STATUS_SUCCESS;
}

//
//	���û�̬���ں�̬�Ͽ�����ʱ����õ��ĺ���
//
VOID
NPMiniDisconnect(
    __in_opt PVOID ConnectionCookie
   )
{		
    PAGED_CODE();
    UNREFERENCED_PARAMETER( ConnectionCookie );
		DbgPrint("[mini-filter] NPMiniDisconnect\n");
    
    //  Close our handle
    FltCloseClientPort( gFilterHandle, &gClientPort );
}

//
//	���û�̬���ں�̬��������ʱ�ں�̬����õ��ĺ���
//
NTSTATUS
NPMiniMessage (
    __in PVOID ConnectionCookie,
    __in_bcount_opt(InputBufferSize) PVOID InputBuffer,
    __in ULONG InputBufferSize,
    __out_bcount_part_opt(OutputBufferSize,*ReturnOutputBufferLength) PVOID OutputBuffer,
    __in ULONG OutputBufferSize,
    __out PULONG ReturnOutputBufferLength
    )
{
		
    NPMINI_COMMAND command;
    NTSTATUS status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( ConnectionCookie );
    UNREFERENCED_PARAMETER( OutputBufferSize );
    UNREFERENCED_PARAMETER( OutputBuffer );
    
		DbgPrint("[mini-filter] NPMiniMessage\n");
    
    //                      **** PLEASE READ ****    
    //  The INPUT and OUTPUT buffers are raw user mode addresses.  The filter
    //  manager has already done a ProbedForRead (on InputBuffer) and
    //  ProbedForWrite (on OutputBuffer) which guarentees they are valid
    //  addresses based on the access (user mode vs. kernel mode).  The
    //  minifilter does not need to do their own probe.
    //  The filter manager is NOT doing any alignment checking on the pointers.
    //  The minifilter must do this themselves if they care (see below).
    //  The minifilter MUST continue to use a try/except around any access to
    //  these buffers.    

    if ((InputBuffer != NULL) &&
        (InputBufferSize >= (FIELD_OFFSET(COMMAND_MESSAGE,Command) +
                             sizeof(NPMINI_COMMAND)))) {

        try  {
            //  Probe and capture input message: the message is raw user mode
            //  buffer, so need to protect with exception handler
            command = ((PCOMMAND_MESSAGE) InputBuffer)->Command;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return GetExceptionCode();
        }

        switch (command) {
            //	ͨ��
            case ENUM_PASS:
            {            		
            		DbgPrint("[mini-filter] ENUM_PASS");
								gCommand = ENUM_PASS;
								status = STATUS_SUCCESS;         
            		break;
            }
            //	��ֹ
            case ENUM_BLOCK:
            {            		
            		DbgPrint("[mini-filter] ENUM_BLOCK");
								gCommand = ENUM_BLOCK;
								status = STATUS_SUCCESS;                          
            		break;
            }               		
            
            default:
            		DbgPrint("[mini-filter] default");
                status = STATUS_INVALID_PARAMETER;
                break;
        }
    } else {

        status = STATUS_INVALID_PARAMETER;
    }

    return status;
}