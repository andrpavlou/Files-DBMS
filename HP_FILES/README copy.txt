-CLOSE() SE COMM STHN MAIN
-STRUCT STHN HP.C v
-HP INFO* (SYNTAX GIA POINTER)ν
-TEST XWRIS DIKH MAS MAIN ν
- BLOXK INFO STHN ARXH TOU BLOCK ν
-

ΜΕΛΗ ΟΜΑΔΑΣ:
-Μικαέλα Κουλλόλλι sdi2100071 
-Αντρεας Παύλου sdi2100223

1)Τα test για την σωστη λειτουργια του κωδικα εγιναν με βαση την main που μας δωθηκε
προσαρμοζωντας την σταδιακα , οποτε δεν φτιαξαμε καποια δικη μας main 
2)Καποιες επισημανσεις οσον αφορα τον κωδικα:
	-To struct HP_BLOCK_INFO που ζητηθηκε να υλοποιήσουμε ειναι υλοποιημενο στο
	αρχειο hp_file.c.
	-Επισης επιλεξαμε να αποθηκευσουμε το block_info του καθε block στην αρχη του
	καθε μπλοκ αντι για το τελος.
	-Μια μικρη συντακτικη παραλλαγη ειναι πχ στην HP_OPEN_FILE δινεται σαν σαν προ-
	τυπο :  HP_info* hpInfo;    
    		return hpInfo;
	ενω εμεις επιλεξαμε:	HP_info info;
  				HP_info* pinfo = &info;
				return pinfo;
	το οποιο καναμε και στις υπολοιπες συναρτησεις για να εχουμε δεικτη στην δομη
	-Στην main δεν κανουμε BF_CLOSE() διοτι το κανουμε στην HP_CLOSEFILE.
	-Δεν εγινε χρηση memcpy() το καναμε manually.