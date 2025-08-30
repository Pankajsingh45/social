#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#define MAX_USERS 100
#define HASH_SIZE 101
#define MAX_MESSAGE_LENGTH 256
#define MAX_GROUPS 10

typedef struct User {
    char username[50];
    char password[50];
    struct User* next;
} User;

typedef struct Friend {
    char username[50];
    struct Friend* next;
} Friend;

typedef struct Message {
    char sender[50];
    char message[256];
    struct Message* next;
} Message;

typedef struct UserNode {
    char username[50];
    Friend* friends;
    Message* messages;
    struct UserNode* next;
} UserNode;

typedef struct GroupMember {
    char username[50];
    struct GroupMember* next;
} GroupMember;

typedef struct Group {
    char groupName[50];
    GroupMember* members;
    Message* messages;
    struct Group* next;
} Group;

User* hashTable[HASH_SIZE];
UserNode* userGraph[MAX_USERS];
Group* groupList = NULL;
int userCount = 0;
int groupCount = 0;

// Utility functions
void trimNewline(char* str) {
    int len = strlen(str);
    if (len > 0 && str[len-1] == '\n') str[len-1] = '\0';
}

void clearScreen() {
#ifdef _WIN32
    Sleep(3000);
    system("cls");
#else
    sleep(3);
    system("clear");
#endif
}

unsigned int hash(char* str) {
    unsigned int hash=0;
    while (*str) hash = (hash << 5) + *str++;
    return hash % HASH_SIZE;
}

void xorCipher(char* message, char* key) {
    size_t keyLen = strlen(key);
    for (size_t i=0; i<strlen(message); i++) {
        message[i] ^= key[i % keyLen];
    }
}

// User functions
int usernameExists(char* username) {
    unsigned int index = hash(username);
    User* current = hashTable[index];
    while (current) {
        if (strcmp(current->username, username) == 0) return 1;
        current = current->next;
    }
    return 0;
}

int registerUser(char* username, char* password) {
    if (usernameExists(username)) {
        printf("Error: Username '%s' already exists.\n", username);
        clearScreen();
        return 0;
    }
    unsigned int index = hash(username);
    User* newUser = (User*)malloc(sizeof(User));
    strcpy(newUser->username, username);
    strcpy(newUser->password, password);
    newUser->next = hashTable[index];
    hashTable[index] = newUser;
    
    UserNode* newNode = (UserNode*)malloc(sizeof(UserNode));
    strcpy(newNode->username, username);
    newNode->friends = NULL;
    newNode->messages = NULL;
    newNode->next = userGraph[userCount];
    userGraph[userCount++] = newNode;
    
    printf("User registered successfully!\n");
    clearScreen();
    return 1;
}

int loginUser(char* username, char* password) {
    unsigned int index = hash(username);
    User* current = hashTable[index];
    while (current) {
        if (strcmp(current->username, username) == 0 && strcmp(current->password, password) == 0) {
            clearScreen();
            return 1;
        }
        current = current->next;
    }
    return 0;
}

// Friend functions
int addFriend(char* user, char* friend) {
    UserNode* userNode = NULL;
    UserNode* friendNode = NULL;
    for (int i=0; i<userCount; i++) {
        if (strcmp(userGraph[i]->username, user) == 0) userNode = userGraph[i];
        if (strcmp(userGraph[i]->username, friend) == 0) friendNode = userGraph[i];
    }
    if (!friendNode) {
        printf("User '%s' doesn't exist.\n", friend);
        clearScreen();
        return 0;
    }
    Friend* temp = userNode->friends;
    while(temp) {
        if (strcmp(temp->username, friend) == 0) {
            printf("You are already friends with %s\n", friend);
            clearScreen();
            return 0;
        }
        temp = temp->next;
    }
    Friend* newFriend1 = (Friend*)malloc(sizeof(Friend));
    strcpy(newFriend1->username, friend);
    newFriend1->next = userNode->friends;
    userNode->friends = newFriend1;
    
    Friend* newFriend2 = (Friend*)malloc(sizeof(Friend));
    strcpy(newFriend2->username, user);
    newFriend2->next = friendNode->friends;
    friendNode->friends = newFriend2;
    
    printf("%s and %s are now friends!\n", user, friend);
    clearScreen();
    return 1;
}

void removeFromFriendList(UserNode* user, char* friendName) {
    Friend* current = user->friends;
    Friend* previous = NULL;
    while (current) {
        if (strcmp(current->username, friendName) == 0) {
            if (previous) previous->next = current->next;
            else user->friends = current->next;
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

int removeFriend(char* user, char* friend) {
    UserNode* userNode=NULL;
    UserNode* friendNode=NULL;
    for (int i=0; i<userCount; i++) {
        if (strcmp(userGraph[i]->username, user) == 0) userNode = userGraph[i];
        if (strcmp(userGraph[i]->username, friend) == 0) friendNode = userGraph[i];
    }
    if (!friendNode) {
        printf("User '%s' doesn't exist.\n", friend);
        clearScreen();
        return 0;
    }
    Friend* temp = userNode->friends;
    int found = 0;
    while(temp) {
        if (strcmp(temp->username, friend) == 0) {
            found = 1;
            break;
        }
        temp = temp->next;
    }
    if (!found) {
        printf("'%s' is not in your friend list.\n", friend);
        clearScreen();
        return 0;
    }
    removeFromFriendList(userNode, friend);
    removeFromFriendList(friendNode, user);
    
    printf("%s and %s are no longer friends.\n", user, friend);
    clearScreen();
    return 1;
}

// Messaging functions
void sendMessage(char* sender, char* receiver, char* message) {
    for (int i=0; i<userCount; i++) {
        if (strcmp(userGraph[i]->username, receiver) == 0) {
            Message* newMessage = (Message*)malloc(sizeof(Message));
            strcpy(newMessage->sender, sender);
            strcpy(newMessage->message, message);
            newMessage->next = NULL;
            if (userGraph[i]->messages == NULL) {
                userGraph[i]->messages = newMessage;
            } else {
                Message* temp = userGraph[i]->messages;
                while(temp->next) temp=temp->next;
                temp->next = newMessage;
            }
            printf("Message sent to %s\n", receiver);
            clearScreen();
            return;
        }
    }
    printf("Receiver not found!\n");
    clearScreen();
}

void displayFriends(char* username) {
    for (int i=0; i<userCount; i++) {
        if (strcmp(userGraph[i]->username, username) == 0) {
            Friend* currentFriend = userGraph[i]->friends;
            printf("Friends of %s:\n", username);
            if (!currentFriend) printf("No friends found.\n");
            while(currentFriend) {
                printf("%s\n", currentFriend->username);
                currentFriend = currentFriend->next;
            }
            clearScreen();
            return;
        }
    }
    printf("User not found!\n");
    clearScreen();
}

void displayMessages(char* username) {
    for (int i=0; i<userCount; i++) {
        if (strcmp(userGraph[i]->username, username) == 0) {
            Message* current = userGraph[i]->messages;
            printf("Messages for %s:\n", username);
            if (!current) printf("No messages found.\n");
            while(current) {
                char decrypted[256];
                strcpy(decrypted, current->message);
                xorCipher(decrypted, current->sender);
                printf("From %s: %s\n", current->sender, decrypted);
                current = current->next;
            }
            int ch=0;
            do {
                printf("Enter 1 to return to home.\n");
                scanf("%d", &ch);
                getchar();
            } while(ch!=1);
            clearScreen();
            return;
        }
    }
    printf("User not found!\n");
    clearScreen();
}

// Group functions
int createGroup(char* creator, char* groupName) {
    Group* current = groupList;
    while(current) {
        if (strcmp(current->groupName, groupName) == 0) {
            printf("Group '%s' already exists.\n", groupName);
            clearScreen();
            return 0;
        }
        current=current->next;
    }
    if (groupCount >= MAX_GROUPS) {
        printf("Maximum group limit reached.\n");
        clearScreen();
        return 0;
    }
    Group* newGroup = (Group*)malloc(sizeof(Group));
    strcpy(newGroup->groupName, groupName);
    newGroup->members = NULL;
    newGroup->messages = NULL;
    newGroup->next = groupList;
    groupList = newGroup;
    GroupMember* creatorMember = (GroupMember*)malloc(sizeof(GroupMember));
    strcpy(creatorMember->username, creator);
    creatorMember->next = newGroup->members;
    newGroup->members = creatorMember;
    groupCount++;
    printf("Group '%s' created successfully!\n", groupName);
    clearScreen();
    return 1;
}

int addMemberToGroup(char* groupName, char* username) {
    Group* group = groupList;
    while(group && strcmp(group->groupName, groupName) != 0) {
        group = group->next;
    }
    if (!group) {
        printf("Group '%s' does not exist.\n", groupName);
        clearScreen();
        return 0;
    }
    GroupMember* member = group->members;
    while(member) {
        if (strcmp(member->username, username) == 0) {
            printf("User '%s' is already a member of group '%s'.\n", username, groupName);
            clearScreen();
            return 0;
        }
        member = member->next;
    }
    GroupMember* newMember = (GroupMember*)malloc(sizeof(GroupMember));
    strcpy(newMember->username, username);
    newMember->next = group->members;
    group->members = newMember;
    printf("User '%s' added to group '%s'.\n", username, groupName);
    clearScreen();
    return 1;
}

void sendGroupMessage(char* sender, char* groupName, char* message) {
    Group* group = groupList;
    while(group && strcmp(group->groupName, groupName) != 0) {
        group = group->next;
    }
    if (!group) {
        printf("Group '%s' does not exist.\n", groupName);
        clearScreen();
        return;
    }
    GroupMember* member = group->members;
    int isMember = 0;
    while(member) {
        if (strcmp(member->username, sender) == 0) {
            isMember = 1;
            break;
        }
        member = member->next;
    }
    if (!isMember) {
        printf("You are not a member of group '%s'.\n", groupName);
        clearScreen();
        return;
    }
    char encryptedMessage[256];
    strcpy(encryptedMessage, message);
    xorCipher(encryptedMessage, sender);
    Message* newMessage = (Message*)malloc(sizeof(Message));
    strcpy(newMessage->sender, sender);
    strcpy(newMessage->message, encryptedMessage);
    newMessage->next = group->messages;
    group->messages = newMessage;
    member = group->members;
    while(member) {
        if (strcmp(member->username, sender) != 0) {
            sendMessage(sender, member->username, encryptedMessage);
        }
        member = member->next;
    }
    printf("Group message sent to '%s'.\n", groupName);
    clearScreen();
}

void displayGroupMessages(char* groupName) {
    Group* group = groupList;
    while(group && strcmp(group->groupName, groupName) != 0) {
        group = group->next;
    }
    if (!group) {
        printf("Group '%s' does not exist.\n", groupName);
        clearScreen();
        return;
    }
    printf("Messages in group '%s':\n", groupName);
    Message* current = group->messages;
    if (!current) printf("No messages in group '%s'.\n", groupName);
    while(current) {
        char decrypted[256];
        strcpy(decrypted, current->message);
        xorCipher(decrypted, current->sender);
        printf("From %s: %s\n", current->sender, decrypted);
        current = current->next;
    }
    int ch = 0;
    do {
        printf("Enter 1 to return to home.\n");
        scanf("%d", &ch);
        getchar();
    } while(ch != 1);
    clearScreen();
}

// Main

int main() {
    int choice;
    char username[50], password[50], friend[50], groupName[50], message[MAX_MESSAGE_LENGTH];

    while (1) {
        printf("\033[1;31m");
        printf("==================================\n");
        printf("|          \\     /               |\n");
        printf("|           \\ _ /                |\n");
        printf("|         --('v')--              |\n");
        printf("|          ((   ))               |\n");
        printf("|          --\"-\"--               |\n");
        printf("|   Welcome to Zingle-Zingle     |\n");
        printf("==================================\n");
        printf("\033[0m");
        
        printf("1. Login\n2. New User Registration\n3. Exit\n\n");
        printf("Your Choice: ");
        scanf("%d", &choice);
        getchar();

        if(choice == 1) {
            printf("Enter username: ");
            fgets(username, sizeof(username), stdin);
            trimNewline(username);
            printf("Enter password: ");
            fgets(password, sizeof(password), stdin);
            trimNewline(password);

            if(loginUser(username, password)) {
                printf("Login successful!\n");
                int userChoice;
                while(1) {
                    printf("1. Add Friend\n2. Remove Friend\n3. Message\n4. Display Friends\n5. Display Messages\n6. Create Group\n7. Add Member to Group\n8. Send Group Message\n9. Display Group Messages\n10. Logout\n");
                    scanf("%d", &userChoice);
                    getchar();

                    if(userChoice == 10) {
                        clearScreen();
                        break;
                    }

                    switch(userChoice) {
                        case 1:
                            printf("Enter friend's username: ");
                            fgets(friend, sizeof(friend), stdin);
                            trimNewline(friend);
                            addFriend(username, friend);
                            break;

                        case 2:
                            printf("Enter friend's username to remove: ");
                            fgets(friend, sizeof(friend), stdin);
                            trimNewline(friend);
                            if(removeFriend(username, friend)) {
                                printf("Friend removed successfully!\n");
                            } else {
                                printf("Friend not found!\n");
                            }
                            break;

                        case 3:
                            printf("Enter receiver's username: ");
                            fgets(friend, sizeof(friend), stdin);
                            trimNewline(friend);
                            printf("Enter message: ");
                            fgets(message, sizeof(message), stdin);
                            trimNewline(message);
                            xorCipher(message, username);
                            sendMessage(username, friend, message);
                            break;

                        case 4:
                            displayFriends(username);
                            break;

                        case 5:
                            displayMessages(username);
                            break;

                        case 6:
                            printf("Enter group name: ");
                            fgets(groupName, sizeof(groupName), stdin);
                            trimNewline(groupName);
                            createGroup(username, groupName);
                            break;

                        case 7:
                            printf("Enter group name: ");
                            fgets(groupName, sizeof(groupName), stdin);
                            trimNewline(groupName);
                            printf("Enter username to add: ");
                            fgets(friend, sizeof(friend), stdin);
                            trimNewline(friend);
                            addMemberToGroup(groupName, friend);
                            break;

                        case 8:
                            printf("Enter group name: ");
                            fgets(groupName, sizeof(groupName), stdin);
                            trimNewline(groupName);
                            printf("Enter message: ");
                            fgets(message, sizeof(message), stdin);
                            trimNewline(message);
                            sendGroupMessage(username, groupName, message);
                            break;

                        case 9:
                            printf("Enter group name: ");
                            fgets(groupName, sizeof(groupName), stdin);
                            trimNewline(groupName);
                            displayGroupMessages(groupName);
                            break;

                        default:
                            printf("Invalid choice!\n");
                            clearScreen();
                    }
                }
            }
            else {
                printf("Invalid username or password!\n");
                clearScreen();
            }
        }
        else if(choice == 2) {
            printf("Enter username: ");
            fgets(username, sizeof(username), stdin);
            trimNewline(username);
            printf("Enter password: ");
            fgets(password, sizeof(password), stdin);
            trimNewline(password);
            registerUser(username, password);
        }
        else if(choice == 3) {
            break;
        }
        else {
            printf("Invalid choice!\n");
            clearScreen();
        }
    }
    return 0;
}

