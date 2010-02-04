namespace acl_update{

void acl_update_start();

void acl_update_stop();

DWORD WINAPI acl_update_thread(void* arg);

}