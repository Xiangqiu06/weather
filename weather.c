#include"head.h"

#define USR_APPKEY          "79731"          
#define USR_SIGN            "f8d89dab4c28c62b4268460b9047dac1"      

#define NOWAPI_SERADDR      "8.163.63.15"
#define NOWAPI_PORT         80

typedef struct weather
{
    char days[32];
    char temperature[32];
    char humidity[32];
    char aqi[32];
    char weather[32];

}weather_t;

int creatTcpConnection(const char*pIP,int Port)
{
    int sockfd;
    struct sockaddr_in recvaddr;
    int ret = 0;
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        perror("fail to socket");
        return -1;
    }
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_port = htons(Port);
    recvaddr.sin_addr.s_addr = inet_addr(pIP);
    ret = connect(sockfd,(struct sockaddr *)&recvaddr,sizeof(recvaddr));
    if(ret == -1)
    {
        perror("fail to connect");
        return -1;
    }
    return sockfd;
}


int SendHttpRequest(int sockfd,char *pUrl)
{
    char tmpbuff[4096] = {0};
    ssize_t nret;
    sprintf(tmpbuff, "GET %s HTTP/1.1\r\n", pUrl);
    sprintf(tmpbuff, "%sHost: api.k780.com\r\n", tmpbuff);
    sprintf(tmpbuff, "%sUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/113.0\r\n", tmpbuff);
    sprintf(tmpbuff, "%sAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n", tmpbuff);
    sprintf(tmpbuff, "%sAccept-Language: en-US,en;q=0.5\r\n", tmpbuff);
    sprintf(tmpbuff, "%sAccept-Encoding: gzip, deflate\r\n", tmpbuff);
    sprintf(tmpbuff, "%sConnection: keep-alive\r\n", tmpbuff);
    sprintf(tmpbuff, "%s\r\n", tmpbuff);
    nret = send(sockfd,tmpbuff,strlen(tmpbuff),0);
    if(nret == -1)
    {
        perror("fail to send");
        return -1;
    }
    return 0;
}

int RecvHttpRespone(int sockfd,char *Recvbuff,int Maxlen)
{
    ssize_t nret = 0;

    nret = recv(sockfd,Recvbuff,Maxlen,0);
    if(nret == -1)
    {
        perror("fail to recv");
        return -1;
    }
    return nret;

}

int handlestr(char *Recvbuff,char *storagebuffer)
{
    int i = 0;
    char *ptemp = NULL;
    char *pjason_start = NULL;
    char *pjason_end = NULL;
    ptemp = strstr(Recvbuff,"\r\n\r\n");
    if(ptemp != NULL)
    {
        pjason_start = strchr(ptemp,'{');
        if(pjason_start == NULL)
        {
            return -1;
        }
    }
    pjason_end = strstr(pjason_start,"\r\n");
    if(pjason_end != NULL)
    {
        pjason_end--;
    }
    else
    {
        return -1;
    }

    while(pjason_start <= pjason_end)
    {
        storagebuffer[i] =  *pjason_start;
        pjason_start++;
        i++;
        
    }
    storagebuffer[i] = '\0';
    return 0;


}

weather_t JsonTool(char *storagebuffer)
{
    cJSON *Object = NULL;
    cJSON *Value = NULL;
    weather_t cur_weather;
    //解析json字符串
    Object = cJSON_Parse(storagebuffer);
    if(Object == NULL)
    {
        printf("fail to cJSON_Parse\n");
        return ;
    }
    //按键名提取键值
    Value = cJSON_GetObjectItem(Object,"result");
    if(Value != NULL)
    {
        cJSON *aqi =cJSON_GetObjectItem(Value,"aqi");
        cJSON *days =cJSON_GetObjectItem(Value,"days");
        cJSON *humidity =cJSON_GetObjectItem(Value,"humidity");
        cJSON *temperature =cJSON_GetObjectItem(Value,"temperature");
        cJSON *weather =cJSON_GetObjectItem(Value,"weather");

        if(cJSON_IsString(aqi))
        {
            strcpy(cur_weather.aqi ,aqi->valuestring);
        }

        if(cJSON_IsString(days))
        {
            strcpy(cur_weather.days ,days->valuestring);
        }

        if(cJSON_IsString(humidity))
        {
            strcpy(cur_weather.humidity ,humidity->valuestring);
        }

        if(cJSON_IsString(temperature))
        {
            strcpy(cur_weather.temperature ,temperature->valuestring);
        }

        if(cJSON_IsString(weather))
        {
            strcpy(cur_weather.weather ,weather->valuestring);
        }

    }
    
    //释放cjson结构体内存
    cJSON_Delete(Object);

    return cur_weather;

}

int main(void)
{
    char CityName[32] = {0};
    int sockfd;
    char Url[256] = {0};
    char Recvbuff[4096] = {0};
    char storagebuffer[4096] = {0};
    char *p = NULL;
    weather_t curweather;
    //获取城市
    printf("请输入城市\n");
    gets(CityName);
    //创建套接字
    sockfd = creatTcpConnection(NOWAPI_SERADDR,NOWAPI_PORT);
    if (-1 == sockfd)
    {
        fprintf(stderr, "CreateTcpConnection failed\n");
        return -1;
    }
    //拼接获取资源路径
    sprintf(Url,"/?app=weather.today&weaid=%s&appkey=%s&sign=%s&format=json",CityName,USR_APPKEY,USR_SIGN);
    //发送报文请求
    SendHttpRequest(sockfd,Url);
    //接收报文响应
    RecvHttpRespone(sockfd,Recvbuff,sizeof(Recvbuff));

    handlestr(Recvbuff,storagebuffer);

    
    curweather = JsonTool(storagebuffer);
    printf("aqi = %s\n",curweather.aqi);
    printf("days = %s\n",curweather.days);
    printf("humidity = %s\n",curweather.humidity);
    printf("temperature = %s\n",curweather.temperature);
    printf("weather = %s\n",curweather.weather);


    



#if 0
    printf("============================ RECV ===============================\n");
    printf("%s", Recvbuff);
    printf("=================================================================\n");
#endif

    close(sockfd);
    return 0;
}

