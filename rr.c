#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;
  bool hit;
  u32 net;
  TAILQ_ENTRY(process) pointers;
};

TAILQ_HEAD(process_list, process); //head of linked list

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path, struct process **process_data, u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list); //initialize process list

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  for(int i = 0; i < size; i++){
    data[i].hit = false;
    data[i].net = data[i].arrival_time - data[i].burst_time;
  }

  int timer = 0;
  int num_processes = size;
  while (true){
    //implement a round robin scheduler
    for (int i = 0; i < size; i++){
      if (data[i].arrival_time > (timer + quantum_length)&& data[i].arrival_time <= timer ){
        printf("proc %d queued on or before %d\n",data[i].pid,timer);
        TAILQ_INSERT_TAIL(&list, &data[i], pointers);
        printf("proc %d arrives at %d\n",data[i].pid,data[i].arrival_time);
      }
     
    }
    struct process *current = TAILQ_FIRST(&list);
    TAILQ_REMOVE(&list, current, pointers);
    num_processes--;
    if(current->hit == false){
      total_response_time += (timer - current->arrival_time);
      current->hit = true;
    }

    if (current->burst_time > quantum_length){
      printf("pid %d\n",current->pid);
      timer += quantum_length;
      printf("timer %d\n",timer);
      current->burst_time -= quantum_length;
      printf("burst time %d\n",current->burst_time);
      TAILQ_INSERT_TAIL(&list, current, pointers);
      num_processes++;
    } else {
      printf("pid %d\n",current->pid);
      timer += current->burst_time;
      printf("timer %d\n",timer);
      int a = timer - current->net;
      printf("curr proc wait time %d\n",a);
      total_waiting_time += (timer - current->net);
      printf("tot wait %d\n",total_waiting_time);
      printf("avg wait %.2f\n", (float)total_waiting_time / (float)size);
      num_processes--;
    }

    if (num_processes == 0){
      break;
    }
  }
  
  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size); //WORKING!
  free(data);
  return 0;
}
