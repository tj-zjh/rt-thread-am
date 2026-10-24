#include <am.h>
#include <klib.h>
#include <rtthread.h>

static Context **target, **source;

static Context* ev_handler(Event e, Context *c) {
  switch (e.event) {
    case EVENT_YIELD:
      if (source != NULL) *source = c;
      return *target;
      
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return c;
}

struct ThreadArgs {
  void (*tentry)(void *);
  void * parameter;
  void (*texit)(void);
};

void wrapper(void *t) {
  struct ThreadArgs *p = t;
  p->tentry(p->parameter);
  p->texit();
}

void __am_cte_init() {
  cte_init(ev_handler);
}



void rt_hw_context_switch_to(rt_ubase_t to) {
  target = (Context **)to;
  source = NULL;
  yield();
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  source = (Context **)from;
  target = (Context **)to;
  yield();
}

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
  uintptr_t stack_align_addr = RT_ALIGN_DOWN((uintptr_t)stack_addr, sizeof(uintptr_t));
  struct ThreadArgs *Thread_addr = (struct ThreadArgs *)(stack_align_addr - sizeof(struct ThreadArgs));
  Thread_addr->tentry = tentry;
  Thread_addr->parameter = parameter;
  Thread_addr->texit = texit;
  Area area;
  area.start = NULL;
  area.end = Thread_addr;
  Context *c = kcontext(area, wrapper, Thread_addr);
  return (rt_uint8_t *)c;
}
