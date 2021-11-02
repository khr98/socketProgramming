#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // 읽기, 쓰기를 위한 헤더
#include <string.h>
#include <netinet/in.h> // 소켓을 만들기 위한 헤더
#include <errno.h>
#include <signal.h>

#define BACKLOG 5

// File = gif, png, mp3, html, pdf
int main(int argc, char *argv[]) {

	int cli_fd, ser_fd;
	
	char request[256];
	char response[256];
	
	struct sockaddr_in serv_addr, cli_addr;
	int sock_n; // 소켓 number

	// 포트넘버가 들어오지 않았다면 에러출력.
	if (argc < 2) {
		fprintf(stderr,"Error! Input port number!\n");
		exit(1);
	}

	// 소켓 생성 파트
	// int socket(int domain, int type, int protocol)
	// 소켓생성에 성공하면 소켓 디스크립터 반환, 오류시 -1 반환.
	cli_fd = socket(PF_INET, SOCK_STREAM, 0); 
	// PF_INET : IP버전4 프로토콜을 사용하자
	// SOCK_STREAM : TCP를 사용
	if (cli_fd <0) {
		perror("Error! making socket failed!\n");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;		// IPv4
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // 서버 ip 지정
	serv_addr.sin_port = htons(atoi(argv[1]));  // 입력값으로 서버 port 지정

	// 소켓에 서버 주소를 할당한다.
	// int bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))
	if (bind(cli_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("Error! binding failed!\n");
		exit(1);
	}

	// 연결 요청 대기 상태
 	listen(cli_fd, BACKLOG);

	// 연결 요청 수락하기
	// int accept(int s, struct sockaddr *addr, int *addrlen)
	// 서버의 소켓을 만들면서 연결 요청을 받고, 서버의 소켓 디스크립터를 반환함.
	socklen_t len = sizeof(cli_addr);
	ser_fd = accept(cli_fd, (struct sockaddr *) &cli_addr, &len);
	if (ser_fd < 0) {
		perror("Error! accept failed!\n");
		exit(1);
	}

	// Request 버퍼 생성
	bzero(request,256);
	sock_n = read(ser_fd, request, 255);
	if (sock_n < 0) {
		perror("Error! reading socket number!\n");
		exit(1);
	}

	// HTTP 요청 메세지 출력
	printf("HTTP Request Message \n%s\n\n", request);
	// HTTP 응답 메세지 출력
	printf("HTTP Response Message\n");

	// 파일 이름과 타입.
	char *first = strtok(request,"\r\n");
	char *middle = strtok(first," ");
	char *extra = strtok(NULL," ");
	char *version = strtok(NULL," ");
	char *filename = strtok(extra,"/");
	char message[256];
	bzero(message,256);
	FILE * file = fopen(filename,"rb"); // 파일 찾고 열기
	fseek(file,0,SEEK_END);
	long file_size = ftell(file);
	rewind(file);
	strcat(message,version);
	strcat(message," 200 ok \r\nContent-Type: ");
	
	if(strstr(filename,".jpg") != NULL || strstr(filename,".jpeg") != NULL || strstr(filename,".png")) 
		strcat(message," image/jpeg\r\n");
	else if (strstr(filename,".mp3") != NULL)
		strcat(message," audio/mp3\r\n");
	else if (strstr(filename,".pdf") != NULL)
		strcat(message," application/pdf\r\n");
	else if (strstr(filename,".gif") != NULL)
		strcat(message," image/gif\r\n"); 
	else if (strstr(filename,".html") != NULL)
		strcat(message," txt/html\r\n");
	
	strcat(message, "Content-Length: ");
	char s_file_size[10];
	sprintf(s_file_size, "%ld", file_size);
	strcat(message, " ");
	strcat(message, s_file_size);
	strcat(message, "\r\n\r\n");

	char * data = (char *)malloc(sizeof(char) * file_size);
	bzero(data, file_size);
	fread(data, sizeof(char), file_size,file);
	sock_n = write(ser_fd, message, strlen(message));
	printf("%s\n",message);
	sock_n = write(ser_fd, data, file_size);
	printf("%s\n", data);
	free(data);

	// 소켓 닫기.
	close(cli_fd);
	close(ser_fd);

	return 0;
}
