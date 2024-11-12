#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <assert.h>
#include<stdbool.h>
#define MIN1(X,Y,Q,Z)((X < Y) && (X < Q) && (X < Z)) ? TRUE:FALSE
#define MIN2(X,Y,Q,Z)((Y<X) && (Y < Q) && (Y < Z)) ? TRUE:FALSE
#define MIN3(X,Y,Q,Z)((Q < X) && (Q < Y) && (Q < Z)) ? TRUE:FALSE
#define MIN4(X,Y,Q,Z)((Z< X) && (Z < Y) && (Z < Q)) ? TRUE:FALSE

typedef struct packett {
	unsigned int time;
	unsigned char da;
	unsigned char sa;
	unsigned char  prio;
	int data_len;
	unsigned char* data;
	int checksum;
}packet;

typedef struct route_node {
	unsigned char da;
	char output_port;
	struct route_node* left;
	struct route_node* right;
}S_node;

typedef enum Bool {
	TRUE = 1, FALSE = 0
}Bool;

typedef struct pkt_node {
	packet* pkt;
	struct pkt_node* next;
} S_pkt;

typedef struct Out_Qs_mgr {
	struct pkt_node* head_p1;
	struct pkt_node* tail_p1;
	struct pkt_node* head_p0;
	struct pkt_node* tail_p0;
} S_Out_Qs_mgr;

void packet_read(FILE* fp, packet* pkt);
void alloction(packet* pkt);
void packet_write(FILE* fp, const packet* pkt);
Bool checksum_check(const packet* pkt);
S_node* add_route(S_node* root, unsigned char da, char output_port);
S_node* delete_route(S_node* root, unsigned char da);
S_node* search_route(const S_node* root, unsigned char da);
void print_routing_table(const S_node* root);
void enque_pkt(S_Out_Qs_mgr* QM_ptr, packet* pkt);

S_node* build_route_table(FILE* fp, S_node* root);

S_node* new_node(unsigned char da, char output_port);
packet* deque_pkt(S_Out_Qs_mgr* QM_ptr, char priority);
S_node* get_sub_min(S_node* root);
S_Out_Qs_mgr* createQueue(S_Out_Qs_mgr* Qm_ptr);
S_pkt* newqueueNode();
S_Out_Qs_mgr* deque_pop_front(FILE* fp, S_Out_Qs_mgr* d);
void freetree(S_node* node);
void delete();
packet* deque_pkt(S_Out_Qs_mgr* QM_ptr, char priority);

void packet_read(FILE* fp, packet* pkt) {
	char str = NULL;
	if (pkt == NULL)
		pkt = malloc(sizeof(packet));
	(fscanf(fp, "%lu %hhu %hhu %hhu %d", &pkt->time, &pkt->da, &pkt->sa, &pkt->prio, &pkt->data_len));
	(pkt->data) = malloc(sizeof(unsigned char) * pkt->data_len);
	for (int i = 0; i < pkt->data_len; i++)
		fscanf(fp, "%hhu", &(pkt->data[i]));
	fscanf(fp, "%d", &pkt->checksum);
	//fseek(fp, 2, SEEK_CUR);
	if ((str = fgetc(fp)) == EOF)
		pkt = NULL;

}

void alloction(packet* pkt) {
	(pkt->da) = malloc(sizeof(unsigned char));
	(pkt->sa) = malloc(sizeof(unsigned char));
	(pkt->prio) = malloc(sizeof(unsigned char));
	assert(pkt->da, pkt->prio, pkt->sa);
}
void packet_write(FILE* fp, const packet* pkt) {
	int i = 0;
	fprintf(fp, "%lu %hhu %hhu %hhu %d", pkt->time, pkt->da, pkt->sa, pkt->prio, pkt->data_len);
	for (int j = 0; j < pkt->data_len; j++)
		fprintf(fp, " %hhu", pkt->data[i++]);
	fprintf(fp, "\n");
}

Bool checksum_check(const packet* pkt) {
	unsigned char x = 0;
	x = (((pkt->da ^ pkt->sa) ^ pkt->prio) ^ pkt->data_len);
	for (int c = 0; c < pkt->data_len; c++)
		x = (x ^ pkt->data[c]);
	if (x == pkt->checksum)
		return TRUE;
	return FALSE;
}

S_node* add_route(S_node* root, unsigned char da, char output_port) {
	if (root == NULL)
		return(root = new_node(da, output_port));
	if (root->da >= da)
		root->left = add_route(root->left, da, output_port);
	if (da >= root->da)
		root->right = add_route(root->right, da, output_port);
	return(root);
}

S_node* new_node(unsigned char da, char output_port) {
	S_node* newnode = calloc(1, sizeof(S_node));
	newnode->da = da,
		newnode->output_port = output_port;
	return newnode;
}

S_node* delete_route(S_node* root, unsigned char da) {

	S_node* temp = root;
	if (root == NULL)
		return root;

	if (da < root->da)
		root->left = delete_route(root->left, da);


	else if (da > root->da)
		root->right = delete_route(root->right, da);
	else

		if (root->left == NULL && root->right == NULL)
		{
			free(root);
			root = NULL;
		}
		else if (root->left == NULL)
		{
			S_node* temp = root;
			root = root->right;
			free(temp);
		}
		else if (root->right == NULL)
		{
			S_node* temp = root;
			root = root->left;
			free(temp);
		}
		else {
			if (root->right && root->left) {
				temp = get_sub_min(root->right);
				root->da = temp->da;
				root->right = delete_route(root->right, temp->da);
			}
		}
	return root;
}


S_node* get_sub_min(S_node* root) {
	while (root->left != NULL)
		root = root->left;
	return root;
}

void print_routing_table(const S_node* root) {
	S_node* temp = root;
	if (temp != NULL) {
		print_routing_table(temp->left);
		printf("%hhu\n", temp->da);
		print_routing_table(temp->right);
	}
}

S_node* search_route(const S_node* root, unsigned char da) {
	S_node* temp_root = root;
	if (root == NULL)
		return root;

	if (da < temp_root->da)
		temp_root->left = search_route(temp_root->left, da);

	else if (da > temp_root->da)
		temp_root->right = search_route(temp_root->right, da);
	else
		return root;
}

S_node* build_route_table(FILE* fp, S_node* root)
{
	char aORd = 0, str = 0; unsigned char da = 0; char output_port = 0; static int c = 0;
	fscanf(fp, "%c %hhu %hhu", &aORd, &da, &output_port);

	if (c == 0)//if both are null
		root = new_node(da, output_port);

	c++;
	while (((str = fgetc(fp)) != EOF)) {
		fscanf(fp, "%c", &aORd);
		if (aORd == 'a') {
			fscanf(fp, "%hhu %hhu", &da, &output_port);
			root = add_route(root, da, output_port);
		}
		if (aORd == 'd') {
			fscanf(fp, "%hhu", &da);
			root = delete_route(root, da);
		}
	}

	if (c == 1)

		return root;
}

void enque_pkt(S_Out_Qs_mgr* QM_ptr, packet* pkt) {

	if (QM_ptr == NULL) {
		QM_ptr = malloc(sizeof(S_Out_Qs_mgr));
		QM_ptr->head_p0 = QM_ptr->tail_p0 = NULL;
		QM_ptr->head_p1 = QM_ptr->tail_p1 = NULL;
	}
	packet* pktt = (packet*)malloc(sizeof(packet));
	pktt = pkt;
	S_pkt* new_node = NULL;

	new_node = newqueueNode();
	new_node->pkt = pktt;

	if ((checksum_check(pkt) == FALSE)) {
		free(pkt);
		pkt = NULL; return NULL;
	}
	else {

		if (pkt->prio == 0) {
			if (QM_ptr->tail_p0 == NULL) {
				QM_ptr->head_p0 = new_node;
				QM_ptr->tail_p0 = new_node;
			}
			else
			{

				QM_ptr->tail_p0->next = new_node;//adding at the end
				QM_ptr->tail_p0 = new_node;//changing tail
			}
		}
		else
		{
			if (QM_ptr->head_p1 == NULL) {
				QM_ptr->head_p1 = new_node;
				QM_ptr->tail_p1 = new_node;
			}
			else {
				if (QM_ptr->tail_p1 != NULL) {
					QM_ptr->tail_p1->next = new_node;//adding at the end
					QM_ptr->tail_p1 = new_node;//
				}
			}
		}
	}
	return QM_ptr;
}
packet* deque_pkt(S_Out_Qs_mgr* QM_ptr, char priority) {
	packet* Packet_data = malloc(sizeof(packet));;
	S_pkt* tmp = malloc(sizeof(S_pkt));
	if (priority == 0) { // 0
		if (QM_ptr == NULL)
			return NULL;
		tmp = QM_ptr->head_p0;
		QM_ptr->head_p0 = tmp->next;
		if (QM_ptr->head_p0 == NULL)
			QM_ptr->tail_p0 = NULL;
		Packet_data = tmp->pkt;
		return  Packet_data;
	}
	else
	{
		if (priority == 1) {
			if (QM_ptr == NULL)
				return NULL;
			tmp = QM_ptr->head_p1;
			QM_ptr->head_p1 = tmp->next;
			if (QM_ptr->head_p1 == NULL)
				QM_ptr->tail_p1 = NULL;
			Packet_data = tmp->pkt;
			return  Packet_data;
		}
	}
}
//
//packet* deque_pkt(S_Out_Qs_mgr* QM_ptr, char priority) {
//	if (QM_ptr == NULL)
//		return NULL;
//	else
//	if (QM_ptr != NULL) {
//		if (priority == 0) {
//			return(QM_ptr->head_p0->pkt);
//			QM_ptr->head_p0 = QM_ptr->head_p0->next;
//		}
//		else {
//			return(QM_ptr->head_p1->pkt);
//			QM_ptr->head_p1 = QM_ptr->head_p1->next;
//		}
//	}	
//}
S_Out_Qs_mgr* createQueue(S_Out_Qs_mgr* Qm_ptr)
{
	S_Out_Qs_mgr* q = (S_Out_Qs_mgr*)malloc(sizeof(S_Out_Qs_mgr));
	q->tail_p0 = q->head_p0 = NULL;
	q->tail_p1 = q->head_p1 = NULL;

	return q;
}
S_pkt* newqueueNode() {
	S_pkt* temp = (S_pkt*)malloc(sizeof(S_pkt));
	//temp->next = NULL;
	return temp;
}

void freetree(S_node* node) {
	if (node == NULL)
		return;

	freetree(node->left);
	freetree(node->right);

	free(node);
}

//queueDestroy(S_Out_Qs_mgr *q)
//{
//    while ((q)!=NULL) {
//   	 deque_pkt(q);
//    }
//    free(q);
//}
//S_Out_Qs_mgr *deque_pop_front(FILE*fp,S_Out_Qs_mgr* d) {
//	//S_Out_Qs_mgr v = d->head_p0->val;
//	S_pkt* temp = d->head_p0;
//	if (d->head_p0 == d->tail_p0) {
//		packet_write(fp, d->head_p0->pkt);
//		d->head_p0 = d->tail_p0 = NULL;
//	}
//	else {
//		packet_write(fp, d->head_p0->pkt);
//		d->head_p0 = temp->next;
//	}
//	free(temp);
//	return ;
//}



void main() {
	FILE* fp1 = fopen("port1.txt", "r");
	FILE* fp2 = fopen("port2.txt", "r");
	FILE* fp3 = fopen("port3.txt", "r");
	FILE* fp4 = fopen("port4.txt", "r");
	FILE* fp_route = fopen("route.txt", "r");
	S_node* root = (S_node*)calloc(1, sizeof(S_node)); unsigned char da = 0, output_port = 0;
	packet* pkt = malloc(sizeof(packet)); char key;
	S_Out_Qs_mgr* QM_ptr = malloc(sizeof(S_Out_Qs_mgr));
	S_Out_Qs_mgr* QM_ptr2 = malloc(sizeof(S_Out_Qs_mgr));
	S_Out_Qs_mgr* QM_ptr3 = malloc(sizeof(S_Out_Qs_mgr));
	S_Out_Qs_mgr* QM_ptr4 = malloc(sizeof(S_Out_Qs_mgr));
	S_Out_Qs_mgr* QM_ptr_final = malloc(sizeof(S_Out_Qs_mgr));
	QM_ptr2->head_p0 = QM_ptr2->tail_p0 = NULL;
	QM_ptr2->head_p1 = QM_ptr2->tail_p1 = NULL;
	QM_ptr->head_p0 = QM_ptr->tail_p0 = NULL;
	QM_ptr->head_p1 = QM_ptr->tail_p1 = NULL;
	QM_ptr3->head_p0 = QM_ptr3->tail_p0 = NULL;
	QM_ptr3->head_p1 = QM_ptr3->tail_p1 = NULL;
	QM_ptr4->head_p0 = QM_ptr4->tail_p0 = NULL;
	QM_ptr4->head_p1 = QM_ptr4->tail_p1 = NULL;
	QM_ptr_final->head_p0 = QM_ptr_final->tail_p0 = NULL;
	QM_ptr_final->head_p1 = QM_ptr_final->tail_p1 = NULL;
	int x = 0, c = 0;
	x = 128 ^ 55 ^ 0 ^ 4 ^ 10 ^ 11 ^ 12 ^ 13;
	unsigned int len1 = 0, len2 = 0, len3 = 0, len4 = 0;
	fseek(fp1, 0, SEEK_END);
	len1 = ftell(fp1);
	fseek(fp1, 0, SEEK_SET);

	fseek(fp2, 0, SEEK_END);
	len2 = ftell(fp2);
	fseek(fp2, 0, SEEK_SET);

	fseek(fp3, 0, SEEK_END);
	len3 = ftell(fp3);
	fseek(fp3, 0, SEEK_SET);

	fseek(fp4, 0, SEEK_END);
	len4 = ftell(fp4);
	fseek(fp4, 0, SEEK_SET);

	root = build_route_table(fp_route, root);
	print_routing_table(root);
	//packet* pkt2 = malloc(sizeof(packet));
	c++;
	for (int i = 0; i <= 24; i++) {
		packet* pkt = malloc(sizeof(packet));
		packet_read(fp1, pkt);
		enque_pkt(QM_ptr, pkt);
	}
	for (int i = 0; i <= 24; i++) {
		packet* pkt = malloc(sizeof(packet));
		packet_read(fp2, pkt);
		enque_pkt(QM_ptr2, pkt);
	}
	for (int i = 0; i <= 24; i++) {
		packet* pkt = malloc(sizeof(packet));
		packet_read(fp3, pkt);
		enque_pkt(QM_ptr3, pkt);
	}
	for (int i = 0; i <= 24; i++) {
		packet* pkt = malloc(sizeof(packet));
		packet_read(fp4, pkt);
		enque_pkt(QM_ptr4, pkt);
	}


	// open files
	FILE* fp1_outpurt = fopen("port1_outpurt.txt", "a");
	FILE* fp2_outpurt = fopen("port2_outpurt.txt", "a");
	FILE* fp3_outpurt = fopen("port3_outpurt.txt", "a");
	FILE* fp4_outpurt = fopen("port4_outpurt.txt", "a");
	for (int i = 0; i <= 24 * 4; i++) {
		packet* pkt = malloc(sizeof(packet));
		//printf("%lu %lu %lu %lu \n", QM_ptr->head_p0->pkt->time, QM_ptr2->head_p0->pkt->time, QM_ptr3->head_p0->pkt->time, QM_ptr4->head_p0->pkt->time);
		int asd = MIN1(QM_ptr->head_p0->pkt->time, QM_ptr2->head_p0->pkt->time, QM_ptr3->head_p0->pkt->time, QM_ptr4->head_p0->pkt->time);
		int asd2 = MIN2(QM_ptr->head_p0->pkt->time, QM_ptr2->head_p0->pkt->time, QM_ptr3->head_p0->pkt->time, QM_ptr4->head_p0->pkt->time);
		int asd3 = MIN3(QM_ptr->head_p0->pkt->time, QM_ptr2->head_p0->pkt->time, QM_ptr3->head_p0->pkt->time, QM_ptr4->head_p0->pkt->time);
		int asd4 = MIN4(QM_ptr->head_p0->pkt->time, QM_ptr2->head_p0->pkt->time, QM_ptr3->head_p0->pkt->time, QM_ptr4->head_p0->pkt->time);
		if (asd == TRUE) {


			int res1 = search_route(root, QM_ptr->head_p0->pkt->da);
			if (res1 != NULL) {
			//	free(QM_ptr->head_p0->pkt);
			//}
			//else
			//{
				packet_write(fp1_outpurt, deque_pkt(QM_ptr, QM_ptr->head_p0->pkt->prio));
			}
		}
		if (asd2 == TRUE) {
			int res2 = search_route(root, QM_ptr2->head_p0->pkt->da);
			if (res2 != NULL) {
				//free(QM_ptr2->head_p0->pkt);

			//}
			//else
			//{
				packet_write(fp2_outpurt, deque_pkt(QM_ptr2, QM_ptr2->head_p0->pkt->prio));
			}
		}
		if (asd3 == TRUE) {
			int res3 = search_route(root, QM_ptr3->head_p0->pkt->da);
			if (res3 != NULL) {
				//free(QM_ptr3->head_p0->pkt);

			//}
			//else
			//{
				packet_write(fp3_outpurt, deque_pkt(QM_ptr3, QM_ptr3->head_p0->pkt->prio));
			}
		}
		if (asd4 == TRUE) {
			int res4 = search_route(root, QM_ptr4->head_p0->pkt->da);
			//deque_pkt(QM_ptr4, QM_ptr4->head_p0->pkt->prio);
			if (res4 != NULL) {
				//free(QM_ptr4->head_p0->pkt);
			//}
			//else
			//{
				packet_write(fp4_outpurt, deque_pkt(QM_ptr4, QM_ptr4->head_p0->pkt->prio));
			}
		}
	}
	//close files
	fclose(fp1_outpurt);
	fclose(fp2_outpurt);
	fclose(fp3_outpurt);
	fclose(fp4_outpurt);

	system("pause");
}


/*
		if (asd == TRUE) {
				deque_pkt(QM_ptr, QM_ptr->head_p0->pkt->prio);
				int res1 = search_route(root, QM_ptr->head_p0->pkt->da);
				if (res1 == NULL) {
					free(QM_ptr->head_p0->pkt);
					S_pkt* temp = malloc(sizeof(S_pkt));
					temp = QM_ptr->head_p0;
					QM_ptr->head_p0= QM_ptr->head_p0->next;
				}
				else
				{
					packet_write(fp1, QM_ptr->head_p0->pkt);
					S_pkt* temp = malloc(sizeof(S_pkt));
					temp = QM_ptr->head_p0;
					QM_ptr->head_p0 = QM_ptr->head_p0->next;
				}
			}
			if (asd2==TRUE) {
				int res2 = search_route(root, QM_ptr2->head_p0->pkt->da);
				if (res2 == NULL) {
					free(QM_ptr2->head_p0->pkt);
					S_pkt* temp = malloc(sizeof(S_pkt));
					temp = QM_ptr2->head_p0;
					 QM_ptr2->head_p0= QM_ptr2->head_p0->next;
				}
				else
				{
					packet_write(fp2, QM_ptr2->head_p0->pkt);
					S_pkt* temp = malloc(sizeof(S_pkt));
					temp = QM_ptr2->head_p0;
					 QM_ptr2->head_p0= QM_ptr2->head_p0->next;
				}
			}
			if (asd3 == TRUE) {
				deque_pkt(QM_ptr3, QM_ptr3->head_p0->pkt->prio);
				int res3 = search_route(root, QM_ptr3->head_p0->pkt->da);
				if (res3 == NULL) {
					free(QM_ptr3->head_p0->pkt);
					S_pkt* temp = malloc(sizeof(S_pkt));
					temp = QM_ptr3->head_p0;
					  QM_ptr3->head_p0= QM_ptr3->head_p0->next;
				}
				else
				{
					packet_write(fp3, QM_ptr3->head_p0->pkt);
					S_pkt* temp = malloc(sizeof(S_pkt));
					temp = QM_ptr3->head_p0;
					QM_ptr3->head_p0 = QM_ptr3->head_p0->next;
				}
			}
			if (asd4 == TRUE) {
				deque_pkt(QM_ptr4, QM_ptr4->head_p0->pkt->prio);
				int res4 = search_route(root, QM_ptr4->head_p0->pkt->da);
				if (res4 == NULL) {
					free(QM_ptr4->head_p0->pkt);
					S_pkt* temp = malloc(sizeof(S_pkt));
					temp = QM_ptr4->head_p0;
					 QM_ptr4->head_p0= QM_ptr4->head_p0->next;
				}
				else
				{
					S_pkt* temp = malloc(sizeof(S_pkt));
					packet_write(fp4, QM_ptr4->head_p0->pkt);
					temp = QM_ptr4->head_p0;
					QM_ptr4->head_p0 = QM_ptr4->head_p0->next;
				}
			}
		}


*/