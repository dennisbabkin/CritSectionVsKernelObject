# CritSectionVsKernelObject

This is a repo with the sample code for my blog post:

## [Critical Section vs Kernel Objects](https://dennisbabkin.com/blog/?i=AAA11D00)
### Spinning in user-mode versus entering kernel - the cost of a SYSCALL in Windows.

This POC project demonstrates performance difference between a critical section and a synchronization kernel object in Windows.

![scrsht_01](https://github.com/dennisbabkin/CritSectionVsKernelObject/blob/main/CritSectionVsKernelObject/Screenshots/scr1.png)

It covers the following topics:

- [Intro](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#into)

- [Critical Section](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#cs_lock)

  - [Critical Section Internals](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#cs_internals)

- [Kernel Synchronization Objects](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#kernel_lock)
- [Entering Kernel From a User-Mode](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#enter_kernel)
- [The Cost of a SYSCALL](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#cost_syscall)

  - [Entering SYSCALL](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#enter_syscall)
  - [syscall Instruction](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#syscall)
  - [Beginning of The System Call Handler](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#sys_call_hndlr)
  - [Kernel Stack Layout](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#stack_layout)
  - [KiSystemServiceUser](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#svc_user)
  - [System Service Number](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#sys_srv_num)
  - [Service Descriptor Tables](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#ssvc_desc_tbls)
  - [System Service Number To Service Function](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#sys_srv_num_to_svc_func)
  - [Service Function Input Parameters](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#ssvc_func_params)
  - [Calling The Service Function](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#call_svc_func)

- [Leaving SYSCALL](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#leave_syscall)

  - [Return From The Service Function](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#ret_svc_func)
  - [Processing User-Mode APCs](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#user_apcs)
  - [Security Mitigations At Exit](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#exit_sec_mit)
  - [Instrumentation Callback](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#InstrumentationCallback)
  - [More Security Mitigations At Exit](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#more_exit_sec_mit)
  - [sysretq Instruction](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#sysretq)
  - [System Exit With Meltdown Mitigations](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#sysexit_meltdown)

- [Curious Find](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#curious_find)
- [POC To Illustrate The Performance Impact](https://dennisbabkin.com/blog/?t=critical_section_vs_kernel_objects_in_windows#poc)

