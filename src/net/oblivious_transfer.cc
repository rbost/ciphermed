//
//  oblivious_transfer.cc
//  ciphermed-proj
//
//  Created by Raphael Bost on 31/10/2014.
//
//

#include <net/oblivious_transfer.hh>

#include <math/math_util.hh>
#include <net/net_utils.hh>
#include <net/message_io.hh>

const char* ifcp1024 = "B10B8F96A080E01DDE92DE5EAE5D54EC52C99FBCFB06A3C69A6A9DCA52D23B616073E28675A23D189838EF1E2EE652C013ECB4AEA906112324975C3CD49B83BFACCBDD7D90C4BD7098488E9C219A73724EFFD6FAE5644738FAA31A4FF55BCCC0A151AF5F0DC8B4BD45BF37DF365C1A65E68CFDA76D4DA708DF1FB2BC2E4A4371";//"124325339146889384540494091085456630009856882741872806181731279018491820800119460022367403769795008250021191767583423221479185609066059226301250167164084041279837566626881119772675984258163062926954046545485368458404445166682380071370274810671501916789361956272226105723317679562001235501455748016154805420913";
const char* ifcg1024 = "A4D1CBD5C3FD34126765A442EFB99905F8104DD258AC507FD6406CFF14266D31266FEA1E5C41564B777E690F5504F213160217B4B01B886A5E91547F9E2749F4D7FBD7D3B9A92EE1909D0D2263F80A76A6A24C087A091F531DBF0A0169B6A28AD662A4D18E73AFA32D779D5918D08BC8858F4DCEF97C2A24855E6EEB22B3B2E5";//"115740200527109164239523414760926155534485715860090261532154107313946218459149402375178179458041461723723231563839316251515439564315555249353831328479173170684416728715378198172203100328308536292821245983596065287318698169565702979765910089654821728828592422299160041156491980943427556153020487552135890973413";
const char* ifcq1024 = "F518AA8781A8DF278ABA4E7D64B7CB9D49462353";


const char* ifcp2048 = "AD107E1E9123A9D0D660FAA79559C51FA20D64E5683B9FD1B54B1597B61D0A75E6FA141DF95A56DBAF9A3C407BA1DF15EB3D688A309C180E1DE6B85A1274A0A66D3F8152AD6AC2129037C9EDEFDA4DF8D91E8FEF55B7394B7AD5B7D0B6C12207C9F98D11ED34DBF6C6BA0B2C8BBC27BE6A00E0A0B9C49708B3BF8A317091883681286130BC8985DB1602E714415D9330278273C7DE31EFDC7310F7121FD5A07415987D9ADC0A486DCDF93ACC44328387315D75E198C641A480CD86A1B9E587E8BE60E69CC928B2B9C52172E413042E9B23F10B0E16E79763C9B53DCF4BA80A29E3FB73C16B8E75B97EF363E2FFA31F71CF9DE5384E71B81C0AC4DFFE0C10E64F";//"00b65b726556c9a2402a04b59b68a0cb95da0212463f749b51b5c193b74900bc180346bc26fd0e1e0a8a7401d7bbbeb6af392d5c6d731cb4fad8f9690bd1c4359b8903e922b9da095697734fb70c398578db49449cd9a26171cb477f226c7183aeaef698276a133f2b132d3d7b1ef989f167e4cc5f93c7df56ed1d58c581813918a49a4cbcc80c40a37bc70d8d2710d9496a0c95dd2c54dff7601d40b3537f3cd44c140a8a75e3b2b38cc5a63a7a289fd53642ec198853c9c53608f0ed773499bf93f53dda18c9cbe7aedf64da73adeaae9fe11710ec2584e5d8c249c51a4e7fc7cd76d7fd9253407cfa4c32060e8e7979b7135a413c01b9615dc3386cc9a3709b";
const char* ifcg2048 = "AC4032EF4F2D9AE39DF30B5C8FFDAC506CDEBE7B89998CAF74866A08CFE4FFE3A6824A4E10B9A6F0DD921F01A70C4AFAAB739D7700C29F52C57DB17C620A8652BE5E9001A8D66AD7C17669101999024AF4D027275AC1348BB8A762D0521BC98AE247150422EA1ED409939D54DA7460CDB5F6C6B250717CBEF180EB34118E98D119529A45D6F834566E3025E316A330EFBB77A86F0C1AB15B051AE3D428C8F8ACB70A8137150B8EEB10E183EDD19963DDD9E263E4770589EF6AA21E7F5F2FF381B539CCE3409D13CD566AFBB48D6C019181E1BCFE94B30269EDFE72FE9B6AA4BD7B5A0F1C71CFFF4C19C418E1F6EC017981BC087F2A7065B384B890D3191F2BFA";//"008adeebc1dbabc36b87f623db7b945c15af86cac74eecd9549a18fa14e76278ac94177ee398225b6376806410a18da49c6bea5102a3101c8f43a30a046f59389c6d3d9ca73fe8950c3b02939aa63468c1b1192fabe42f74b9501b1dd5a39d75c9633a60bacd9d2964121853ad9db99667f20728cf0fa0bf40520ee64d733760c8e0ed430c22ce98a1dc0107da70cdf738930d0d878fc9885770ef922e359f9c2960828d09615a50b19ab3630839c74fa939f00fe27d75568a6d3d213d6938d283e244c87a774f50cfa26192965e52687bc8046e940a0fafc558497d2a2f8b33cf1a22a9636f97ac33e8516d676379a016377ea45ae0bb44ccb30c7cb2f85e2ab8";
const char* ifcq2048 = "801C0D34C58D93FE997177101F80535A4738CEBCBF389A99B36371EB";

const char* ifcp3072 = "4660194093823565506151007332698542081380390944320667936220310340292682538415201463451360005469701273992420569531194415296871671272562243754789577412471203509686259933515539120145538889500684305065682267020422897056483203401642088590732633756278140548667640739272073464322452643609409839498807131787408915921523565001045685221279165409792825261753615641493423723165471868882028678262386826730035778207616806238910696112513243832793252430036079010833108716296401084350809152423357477416465451376967706115065572717893335336664895800189754170750266169252030669114411476002012410621336179123441424048589750501111541393610787337793314723136089502117079738181113934544472215273637670210480814609550715859453809706797176331069587697357167970759889883398852942449568449890603652456531060380065260476714266615239827983706919432589669744367350756821903843388105282430635020233707272521317674908786962912228887786913664926989228941514639";//"4789243463423558042555942692067952010762941537156456953540207552393517268106963979997829026263647966345508471957906220742100227273384809716752152112356809825736300757377245802611753896061859326237585053121166511488055185732839278062397760542437485455001142202189435272728709541874806877733850125589928683405291686167533266713936916675782448917821790738415042990061268204756037256482352769204623714637964899766769955563661482997312273357479538189538517145384302735254987990835037196275140131495878169397796729856237474545282819466276030272350690656820774870407765694055030510886563214726305250475193447404448987580428573823087076250936463211266260424571314164485183464492496052322524964994076849736875524485625736125539794888488754272502203109743730459721059579701891700685852089310632886111935746207218212166594263729240408980591141882951231690705885743071588694711524369611401615638599873148777498602296639831856612247665021";
const char* ifcg3072 = "326984479748743614358878489890111032378521682641889472728164592588245254735528952815040417677135099463681521117067228131302984716932197927691804537047698386112034189358693637764887258325546424576668654933254773228919028116187485325776123548207630122958160311311825230114818910264101591293903307807790394765896174615027850669640300925521032111542648598127663424462192520490917608209583615366128345913820058976254028107968965281721876376153097516948596625654797921929621363755081263164203185942482227411046415127689226121648774535224687708280963930985498313715804706762069594298539593719253724193098201932449349224692341850008449711165375995101343314201170357859203662648251088921851885444086613889195257606710405156897225917687758015354941738963422772322756212536951044725465040734436163477969317027796051497934165333064621979305683254912099909723895352817468375097484456065145582788954244042708099846989842764657922387568064";// "2160520890381000056007657643243429873074958704510399432913343295708996752548164839769903341093252559831860900701193192562818688247615415786663735598112460146846795605682700004601839784712647668365990653831938418748005412270836134744826460948248057729380379226401090764535946353265060683782502632969162586096268685152865962222861901267532144917625816876992295984794617310226677399144624190218230425225446163477438826108973805696142676104536334682740626195572874773359182313764494044146238803592517599298886664249718365933398591936954061485060605858266612466425666407440613251131465090157566740372012599597095678634369430696426360778888785815452635230389040236453264600018871371049581621037039225056418281127472896664431348097475952759433511397526843858700296695192163658572951041054351185123394179949273815116087614497977570773174953888408872632726100294942696240394436920568841989388330145750695278253233343153002655304051047";
const char* ifcq3072 = "95729504467608377623766753562217147614989054519467474668915026082895293552781";

#define SHA1_BYTES				20


static void hashReturn(byte* ret, byte* val, int val_len, int ctr) {
    SHA_CTX sha;
    HASH_INIT(&sha);
    HASH_UPDATE(&sha, (char*) val, val_len);
    HASH_UPDATE(&sha, (char*) &ctr, sizeof(int));
    HASH_FINAL(&sha, ret);
    
}


bool ObliviousTransfer::GMP_Init(int secparam) {
    m_SecParam = secparam;
    mpz_init(m_NPState.p);
    mpz_init(m_NPState.g);
    mpz_init(m_NPState.q);
    switch (secparam)
    {
        case 1024:
            mpz_set_str(m_NPState.p, ifcp1024, 16);	mpz_set_str(m_NPState.g, ifcg1024, 16);	mpz_set_str(m_NPState.q, ifcq1024, 16); break;
        case 2048:
            mpz_set_str(m_NPState.p, ifcp2048, 16);	mpz_set_str(m_NPState.g, ifcg2048, 16);	mpz_set_str(m_NPState.q, ifcq2048, 16); break;
        case 3072:
            mpz_set_str(m_NPState.p, ifcp3072, 10);	mpz_set_str(m_NPState.g, ifcg3072, 10);	mpz_set_str(m_NPState.q, ifcq3072, 10); break;
        default:
            mpz_set_str(m_NPState.p, ifcp1024, 16);	mpz_set_str(m_NPState.g, ifcg1024, 16);	mpz_set_str(m_NPState.q, ifcq1024, 16);	m_SecParam = 1024; break;
    }
    
    gmp_randinit_mt (m_NPState.rnd_state );
    m_NPState.field_size = mpz_sizeinbase(m_NPState.p, 2)/8;
    return true;
}

void ObliviousTransfer::mpz_export_padded(char* pBufIdx, int field_size, mpz_t to_export) {
    size_t size = 0;
    mpz_export(pBufIdx, &size, 1, sizeof(pBufIdx[0]), 0, 0, to_export);
    
    if (size < field_size) {
        for (int i = 0; i + size < field_size; i++) {
            pBufIdx[i] = 0;
        }
        pBufIdx += (field_size - size);
        mpz_export(pBufIdx, &size, 1, sizeof(pBufIdx[0]), 0, 0, to_export);
    }
}


bool ObliviousTransfer::GMP_Cleanup()
{
    //TODO: Further cleanup
    gmp_randclear(m_NPState.rnd_state);
    return TRUE;
}




bool ObliviousTransfer::receiver(int nOTs, int *choices, char *ret, tcp::socket &socket)
{
    int nSndVals = 2;
    char* pBuf = new char[nOTs*m_NPState.field_size];
    int nBufSize = nSndVals * m_NPState.field_size;
    
    mpz_t PK_sigma[nOTs], PK0, ztemp, ztmp;
    mpz_t pK[nOTs];
    mpz_t pDec[nOTs];
    mpz_init(PK0);
    mpz_init(ztemp);
    mpz_init(ztmp);
    
    mpz_class PubKey = 0;
    
    FixedPointExp br(m_NPState.g, m_NPState.p, m_NPState.field_size*8);
    
    for (int k = 0; k < nOTs; k++)
    {
        mpz_init(pK[k]);
        mpz_init(pDec[k]);
        mpz_init(PK_sigma[k]);
        
        //generate random PK_sigmas
        mpz_urandomb(ztmp, m_NPState.rnd_state, m_NPState.field_size*8);
        mpz_mod(pK[k], ztmp, m_NPState.q);
        br.powerMod(PK_sigma[k], pK[k]);
    }
    
//    socket.Receive(pBuf, nBufSize);
    readByteStringFromSocket(socket, pBuf, nBufSize);

    char* pBufIdx = pBuf;
    
    mpz_t pC[nSndVals];
    for(int u = 0; u < nSndVals; u++)
    {
        mpz_init(pC[u]);
        mpz_import(pC[u], m_NPState.field_size, 1, sizeof(pBuf[0]), 0, 0, pBufIdx);
        pBufIdx += m_NPState.field_size;
    }
    
    //====================================================
    // N-P receiver: send pk0
    pBufIdx = pBuf;
    int choice;
    for(int k=0; k<nOTs; k++)
    {
        choice = choices[k];
        
        if( choice != 0 )
        {
            mpz_invert(ztmp, PK_sigma[k], m_NPState.p);
            mpz_mul(ztemp, pC[choice], ztmp);
            mpz_mod(PubKey.get_mpz_t(), ztemp, m_NPState.p);
//            mpz_mod(PK0, ztemp, m_NPState.p);
        }
        else
        {
            mpz_set(PubKey.get_mpz_t(), PK_sigma[k]);
//            mpz_set(PK0, PK_sigma[k]);
        }
        
        mpz_export_padded(pBufIdx, m_NPState.field_size, PubKey.get_mpz_t());
//        mpz_export_padded(pBufIdx, m_NPState.field_size, PK0);
        pBufIdx += m_NPState.field_size;
    }
    
//    socket.Send(pBuf, nOTs * m_NPState.field_size);
    writeByteStringFromSocket(socket, pBuf, nOTs * m_NPState.field_size);

    delete pBuf;
    pBuf = new char[m_NPState.field_size];
    char* retPtr = ret;
    // compute masking hashes

    FixedPointExp pbr (pC[0], m_NPState.p, m_NPState.field_size*8);
    for(int k=0; k<nOTs; k++)
    {
        pbr.powerMod(pDec[k], pK[k]);
        mpz_export_padded(pBuf, m_NPState.field_size, pDec[k]);
        hashReturn((byte *)retPtr, (byte *)pBuf, m_NPState.field_size, k);
        retPtr += SHA1_BYTES;
    }
    
    delete [] pBuf;

    char *hashBuf = new char[nOTs * SHA1_BYTES * nSndVals];
    readByteStringFromSocket(socket, hashBuf, nOTs * SHA1_BYTES * nSndVals);

    
    char *hashBufPtr = hashBuf;
    char *chosenHash;
    
    
    
    retPtr = ret;
    for (int k = 0; k < nOTs; k++) {
        choice = choices[k];
        chosenHash = hashBufPtr;
        
//        for (size_t u = 0; i < choice; u++) {
            chosenHash += choice*SHA1_BYTES;
//        }
        
        // unmask the message with the hash
        for (int i = 0; i < SHA1_BYTES; i++) {
            retPtr[i] ^= chosenHash[i];
        }
        
        
        hashBufPtr += nSndVals*SHA1_BYTES;
        retPtr += SHA1_BYTES;
    }

    
    return true;
}




bool ObliviousTransfer::sender(int nOTs, char *messages, tcp::socket &socket)
{
    char* pBuf = new char[m_NPState.field_size * nOTs];
    int nSndVals = 2;

    mpz_t pC[nSndVals], pCr[nSndVals], r, ztmp, ztmp2, PK0r, PKr;
    
    mpz_init(r);
    mpz_init(ztmp);
    mpz_init(ztmp2);
    mpz_init(PK0r);
    mpz_init(PKr);
    
    for(int u = 0; u < nSndVals; u++)
    {
        mpz_init(pC[u]);
        mpz_init(pCr[u]);
    }
    
    //random C1
    mpz_urandomb(ztmp, m_NPState.rnd_state, m_NPState.field_size*8);
    mpz_mod(r, ztmp, m_NPState.q);
    mpz_powm(pC[0], m_NPState.g, r, m_NPState.p);
    
    //random C(i+1)
    for(int u = 1; u < nSndVals; u++)
    {
        mpz_urandomb(ztmp, m_NPState.rnd_state, m_NPState.field_size*8);
        mpz_mod(ztmp2, ztmp, m_NPState.q);
        mpz_powm_ui(pC[u], ztmp2, 2, m_NPState.p);
    }
    
    //====================================================
    // Export the generated C_1-C_nSndVals to a BYTE vector and send them to the receiver
    int nBufSize = nSndVals * m_NPState.field_size;
    char* pBufIdx = pBuf;
    for( int u=0; u<nSndVals; u++ )
    {
        mpz_export_padded(pBufIdx, m_NPState.field_size, pC[u]);
        pBufIdx += m_NPState.field_size;
    }
//    socket.Send(pBuf, nBufSize);
    writeByteStringFromSocket(socket, pBuf, nBufSize);

    //====================================================
    // compute C^R
    for(int u = 1; u < nSndVals; u++)
    {
        mpz_powm(pCr[u], pC[u], r, m_NPState.p);
    }
    
    //====================================================
    // N-P sender: receive pk0
    nBufSize = m_NPState.field_size * nOTs;
//    socket.Receive(pBuf, nBufSize); //receive the d_j's
    readByteStringFromSocket(socket, pBuf, nBufSize);

    pBufIdx = pBuf;
    mpz_t pPK0[nOTs];
    for(int k = 0; k < nOTs; k++)
    {
        mpz_init(pPK0[k]);
        mpz_import(pPK0[k], m_NPState.field_size, 1, sizeof(pBufIdx[0]), 0, 0, pBufIdx);
        pBufIdx += m_NPState.field_size;
    }
    
    delete pBuf;
    pBuf = new char[m_NPState.field_size*nSndVals];
    char *hashBuf = new char[nOTs * SHA1_BYTES * nSndVals];
    char* hashBufPtr = hashBuf;
    char* messagePtr = messages;
    //====================================================
    // Write all nOTs * nSndVals possible values and save hash value to ret
//    char* retPtr= ret;
    for(int k=0; k<nOTs; k++ )
    {
        pBufIdx = pBuf;
        for(int u=0; u<nSndVals; u++)
        {
            if( u == 0 )
            {
                // pk0^r
                mpz_powm(PK0r, pPK0[k], r, m_NPState.p);
                mpz_export_padded(pBufIdx, m_NPState.field_size, PK0r);
                mpz_invert(ztmp, PK0r, m_NPState.p);
            }
            else
            {
                // pk^r
                mpz_mul(ztmp2, pCr[u], ztmp);
                mpz_mod(PKr, ztmp2, m_NPState.p);
                mpz_export_padded(pBufIdx, m_NPState.field_size, PKr);
            }
            
            // compute the hash of PK_u^r
            hashReturn((byte *)hashBufPtr, (byte *)pBufIdx, m_NPState.field_size, k);

            for (int i = 0; i < SHA1_BYTES; i++) {
                hashBufPtr[i] ^= messagePtr[i];
            }
            messagePtr += SHA1_BYTES;
            hashBufPtr += SHA1_BYTES;
            pBufIdx += m_NPState.field_size;
        }
    }
    
    writeByteStringFromSocket(socket, hashBuf, nOTs * SHA1_BYTES * nSndVals);

    delete [] hashBuf;
    delete [] pBuf;

    return true;
}
