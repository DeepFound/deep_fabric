///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#include <stdio.h>
#include <setjmp.h>
#include <string.h>

#include "event.h"
#include "thread.h"
#include "thrdmgr.h"

#if defined TX_JUMP_SUPPORT
	#if defined TX_SOL
		#error "Unconfigured CPU: SOL"
	#endif

	#if defined TX_SUNOS
		#error "Unconfigured CPU: SUNOS"
	#endif

	#if defined TX_SGI
		#error "Unconfigured CPU: SGI"
	#endif

	#if defined TX_HP
		#error "Unconfigured CPU: HP"
	#endif

	#if defined TX_DEC
		#error "Unconfigured CPU: DEC"
	#endif

	#if defined TX_LINUX
		#define JB_SP_INDEX 6
	#endif

	#if defined TX_MAC
		#define JB_SP_INDEX 2
	#endif

	#if defined TX_LINUX
		extern "C"
		{
			extern int home_setjmp(jmp_buf env);
			extern void home_longjmp(jmp_buf env, int val);
		}

		#define SETJMP home_setjmp
		#define LONGJMP home_longjmp
	#else
		#define SETJMP setjmp
		#define LONGJMP longjmp
	#endif

	static const int THREAD_JUMP_CODE = 1;

	class JumpContext
	{
		private:
			Thread* _thread_;
			THREAD_FUNC_PTR _func_;
			void* _args_;

			int _size_;
			void* _stack_;

			long* _getJmpBufStackStart_ (unsigned* stack_base, long stack_size, long frame_size)
			{
				#if defined TX_STACK_UP
					return (long*) (stack_base + frame_size);
				#else
					return (long*) ((stack_base + stack_size) - frame_size);
				#endif
			}

			long _getStackFrameSize_ (long stack_ptr, jmp_buf* buf)
			{
				long* ptr = (long*) buf;
				return abs((long) (stack_ptr - (long) ptr[JB_SP_INDEX]));
			}

			long* _getJmpBufStackPointer_ (jmp_buf* buf)
			{
				long* ptr = (long*) buf;
				return (long*) ptr[JB_SP_INDEX];
			}

			void _setJmpBufStackPointer_ (jmp_buf* buf, long* new_stack_ptr)
			{
				long* ptr = (long*) buf;
				ptr[JB_SP_INDEX] = (long) new_stack_ptr;
			}

		public:
			jmp_buf _jb_;

			JumpContext (Thread* thread, THREAD_FUNC_PTR func, void* args, int size, bool main_thread) :
				_stack_(0), _thread_(thread), _func_(func), _args_(args), _size_(size)
			{
				memset(&_jb_, 0, sizeof(jmp_buf));

				if (main_thread == false)
				{
					_stack_ = malloc(_size_ * sizeof(long));
				}
			}

			~JumpContext (void)
			{
				free(_stack_); _stack_ = 0;
			}

			void init (void)
			{
				long stack_pointer = (long) _getJmpBufStackPointer_(&_jb_);
				long frame_size = _getStackFrameSize_(stack_pointer, &_jb_);
				long* start = _getJmpBufStackStart_((unsigned*) _stack_, _size_, frame_size);
				_setJmpBufStackPointer_(&_jb_, start);
			}

			void execute (void)
			{
				if (_func_)
				{
					_func_(_args_);
					_thread_->kill();
				}
			}
	};
#else
	#if defined TX_LINUX
		#include <ucontext.h>
	#endif

	#if defined TX_WIN
		#define STRICT
		#define _WIN32_WINNT 0x0400

		#include <windows.h>
	#endif

	class FiberContext
	{
		private:
			Thread* _thread_;
			THREAD_FUNC_PTR _func_;
			void* _args_;

			#if defined TX_LINUX
				ucontext_t _uc_;
			#endif

			#if defined TX_WIN
				void* _handle_;
			#endif

		public:
			FiberContext (Thread* thread, THREAD_FUNC_PTR func, void* args, int size, bool main_thread) :
				_thread_(thread), _func_(func), _args_(args)
			{
				#if defined TX_LINUX
					memset(&_uc_, 0, sizeof(_uc_));

					getcontext(&_uc_);

					_uc_.uc_stack.ss_sp = new char [size];
					_uc_.uc_stack.ss_size = size;

					makecontext(&_uc_, (void (*)()) FiberContext::callback, 1, this);
				#endif

				#if defined TX_WIN
					if (main_thread == true)
					{
						_handle_ = ConvertThreadToFiber(0);
					}
					else
					{
						_handle_ = CreateFiber(size,
							(LPFIBER_START_ROUTINE)
								FiberContext::callback, this);
					}
				#endif
			}

			~FiberContext (void)
			{
				#if defined TX_WIN
					if (_handle_)
					{
						DeleteFiber(_handle_);
						_handle_ = 0;
					}
				#endif
			}

			void execute (void)
			{
				if (_func_)
				{
					_func_(_args_);
					_thread_->kill();
				}
			}

			void jump (FiberContext* fiber)
			{
				#if defined TX_LINUX
					if (&fiber->_uc_ != &_uc_)
					{
						swapcontext(&fiber->_uc_, &_uc_);
					}
				#endif

				#if defined TX_WIN
					if (fiber->_handle_ != _handle_)
					{
						SwitchToFiber(_handle_);
					}
				#endif
			}

			static void callback (void* fiber);
	};

	void FiberContext::callback (void* fiber)
	{
		((FiberContext*) fiber)->execute();
	}
#endif

int Thread::_count_ = 0;
int Thread::_max_pri_ = 0;
void* Thread::_new_thread_ = 0;
Thread* Thread::_io_time_thread_ = 0;
Thread* Thread::_current_thread_ = 0;
txList* Thread::_inact_threads_ = 0;
ThreadList** Thread::_act_threads_ = 0;

void Thread::_initThreads_ ()
{
	_act_threads_ = new ThreadList*[THREAD_PRIORITY_RANGE];
	for (int i = 0; i < THREAD_PRIORITY_RANGE; i++)
	{
		_act_threads_[i] = new ThreadList();
	}
}

void Thread::_putThread_ (Thread* t)
{
	int pri = t->priority();

	_act_threads_[pri]->add(t);

	if (pri > _max_pri_)
	{
		_max_pri_ = pri;
	}

	_count_++;
}

Thread* Thread::_getThread_ (void)
{
	register Thread* t = 0;
	register int i = _max_pri_;

	for (; i >= 0 ; i--)
	{
		if (_act_threads_[i]->entries() && (t = (Thread*) _act_threads_[i]->get()))
		{
			break;
		}
	}

	_count_--;

	_max_pri_ = i;
	return t;
}

void Thread::_deleteInactiveThreads_ (void)
{
	Thread* thread;

	while (thread = (Thread*) _inact_threads_->get())
	{
		thread->_deleteContext_();

		if (thread->_mode_ == DYNAMIC)
		{
			delete (Thread*) thread;
		}
	}
}

int Thread::getActiveThreadCount (void)
{
	return _count_;
}

void* Thread::operator new (size_t size)
{
	_new_thread_ = new unsigned char[size];
	return _new_thread_;
}

void Thread::operator delete (void* p)
{
	Thread* t = (Thread*) p;

	t->_changeStateTo_(ZOMBIE);

	_inact_threads_->append((txObject*) t);

	if (t == Thread::_current_thread_)
	{
		t->schedule();
	}
}

Thread::Thread (THREAD_FUNC_PTR func, void* args, const char* name,
	int priority, int size, ThrdState state) :
	_pprev_(0),
	_pnext_(0),
	_eprev_(0),
	_enext_(0),
	_name_(name),
	_wait_event_(0),
	_state_(ZOMBIE),
	_priority_(priority)
{
	if (this == _new_thread_)
	{
		_mode_ = DYNAMIC;
	}
	else
	{
		_mode_ = STATIC;
	}

	if (_act_threads_ == 0)
	{
		_initThreads_();
		_inact_threads_ = new txList();
	}

	_allocContext_(func, args, size);

	_state_ = state;

	if (_state_ == RUNNING)
	{
		Thread::_current_thread_ = this;
	}

	if (_state_ == ACTIVE)
	{
		_putThread_(this);
	}

#if defined TX_NON_PREEMPT_DEBUG
	ThrdMgr::add(this);
#endif
}

Thread::~Thread (void)
{
#if defined TX_NON_PREEMPT_DEBUG
	ThrdMgr::remove(this);
#endif
}

ThrdState Thread::_changeStateTo_ (ThrdState state)
{
	#if defined TX_NON_PREEMPT_STATS
		stats._startIntervalTime_();

		if (state == RUNNING)
		{
			stats._startIntervalCPU_();
		}	
	#endif

	ThrdState old_state = _state_;

	_state_ = state;

	return old_state;
}

void Thread::_deleteContext_ (void)
{
	if (_wait_event_)
	{
		Event::_remove_(_wait_event_, this);
		_wait_event_ = 0;
	}

	delete _context_; _context_ = 0;
}

void Thread::_allocContext_ (THREAD_FUNC_PTR func, void* args, int size)
{
	bool main_thread = (Thread::_current_thread_ == 0);

	#if defined TX_JUMP_SUPPORT
		_context_ = new JumpContext(this, func, args, size, main_thread);

		if (main_thread == true)
		{
			return;
		}

		if (SETJMP(_context_->_jb_) == THREAD_JUMP_CODE)
		{
			Thread::_current_thread_->_context_->execute();
			return;
		}

		_context_->init();
	#else
		_context_ = new FiberContext(this, func, args, size, main_thread);
	#endif
}

void Thread::deactivate (void)
{
	if (_state_ == RUNNING)
	{
		#if defined TX_NON_PREEMPT_STATS
			stats._runCPU_ += stats._getIntervalCPU_();
			stats._runTime_ += stats._getIntervalTime_();

			stats._numYields_++;
		#endif

		_changeStateTo_(DEACTIVE);

		schedule();
	}
}

void Thread::activate (void)
{
	if (_state_ == DEACTIVE)
	{
		#if defined TX_NON_PREEMPT_STATS
			stats._waitTime_ += stats._getIntervalTime_();
		#endif

		_changeStateTo_(ACTIVE);

		_putThread_(this);
	}
}

void Thread::schedule (void)
{
	if (Thread::_current_thread_ == this)
	{
		if (_state_ == RUNNING)
		{
			#if defined TX_NON_PREEMPT_STATS
				stats._runCPU_ += stats._getIntervalCPU_();
				stats._runTime_ += stats._getIntervalTime_();
			#endif

			Thread::_current_thread_->_changeStateTo_(ACTIVE);
			_putThread_(Thread::_current_thread_);
		}

		if (_state_ != ZOMBIE)
		{
			_deleteInactiveThreads_();

			#if defined TX_JUMP_SUPPORT
				if (SETJMP(_context_->_jb_) == THREAD_JUMP_CODE)
				{
					return;
				}
			#endif
		}

		#if defined TX_NON_PREEMPT_STATS
			stats._activeTime_ += stats._getIntervalTime_();
		#endif

		Thread* prev = Thread::_current_thread_;
		Thread::_current_thread_ = _getThread_();
		Thread::_current_thread_->_changeStateTo_(RUNNING);

		#if defined TX_JUMP_SUPPORT
			LONGJMP(Thread::_current_thread_->_context_->_jb_, THREAD_JUMP_CODE);
		#else
			Thread::_current_thread_->_context_->jump(prev->_context_);
		#endif
	}
}

void Thread::kill (void)
{
	delete this;
}

