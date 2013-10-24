#pragma once

#define PORT 1990
#define BASE 62


#define FHE_p 2
#define FHE_r 1;
#define FHE_d 1
#define FHE_c 2;
#define FHE_L 5;
#define FHE_w 64;
#define FHE_s 1;
#define FHE_k 80;
#define FHE_m 0; // XXX: check?

/* Headers for the protocols */

const std::string GET_GM_PK = "GET GM PK";
const std::string END_GM_PK = "END GM PK";
const std::string GM_PK = "GM PK";


const std::string GET_PAILLIER_PK = "GET PAILLIER PK";
const std::string END_PAILLIER_PK = "END PAILLIER PK";
const std::string PAILLIER_PK = "PAILLIER PK";


const std::string GET_FHE_PK = "GET FHE PK";
const std::string END_FHE_PK = "END FHE PK";
const std::string FHE_PK = "FHE PK";

const std::string START_LSIC = "START LSIC";
const std::string LSIC_SETUP = "LSIC SETUP";
const std::string LSIC_END = "LSIC END";
const std::string LSIC_PACKET = "LSIC PACKET";

const std::string DISCONNECT = "DISCONNECT";

const std::string DECRYPT_GM = "DECRYPT GM";
const std::string DECRYPT_FHE = "DECRYPT FHE";

const std::string START_REV_ENC_COMPARE = "START REV ENC COMPARE";
const std::string REV_ENC_COMPARE_CONCLUDE = "REV ENC COMPARE CONCLUDE";
const std::string REV_ENC_COMPARE_RESULT = "REV ENC COMPARE RESULT";
const std::string TEST_REV_ENC_COMPARE = "TEST REV ENC COMPARE";
