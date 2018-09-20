// Copyright (c) 2012-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key.h"

#include "base58.h"
#include "script/script.h"
#include "uint256.h"
#include "util.h"
#include "utilstrencodings.h"

#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

using namespace std;

static const string strSecret1     ("9MD9D2nrcF8nwUURP1tQAdJa3zz1ykcQbnaCbrtziUVtVDP8brLJ");
static const string strSecret2     ("9LbL8DmWXarSRDh3y7S2Kh9cparNEjEKL4QBkPNk7t8ybgPuCUJj");
static const string strSecret1C    ("9NrdYLSvL7s3PxatHCg8RCrfVJwdkbyDhWxiMgLvLhguJTF74Hek");
static const string strSecret2C    ("9KbQjfpbg6NhfvCYmA66VJY2DNNHKX5ccd7EryvBJn3oDs6o2gT9");
static const CBitcoinAddress addr1 ("Zu6B59NFE3562Sor6U8zyVTKh3QZFLcmeA");
static const CBitcoinAddress addr2 ("Zii6T25heWbVjQvbEXcZpxZgkr7nf58iqB");
static const CBitcoinAddress addr1C("ZmqJCtDK7EQSHQXJdjzwYuj21kP6Q2RtPn");
static const CBitcoinAddress addr2C("ZoQsjZfzKYtEfn16YTKSPTrikmqaPbaWTW");


static const string strAddressBad("ZP6Jn5EzUhS2atBtqtQzQn31FVb231LSxH");

#ifdef KEY_TESTS_DUMPINFO
void dumpKeyInfo()
{
    CKey k;
    k.MakeNewKey(false);
    CPrivKey s = k.GetPrivKey();

    printf("Generating new key\n");

    for (int nCompressed=0; nCompressed<2; nCompressed++)
    {
        bool bCompressed = nCompressed == 1;

        CKey key;
        key.SetPrivKey(s, bCompressed);
        CPrivKey secret = key.GetPrivKey();
        CPubKey pubKey = key.GetPubKey();

        CBitcoinSecret bsecret;
        bsecret.SetKey(key);

        printf("  * %s:\n", bCompressed ? "compressed" : "uncompressed");
        printf("    * secret (base58): %s\n", bsecret.ToString().c_str());
        printf("    * pubkey (hex): %s\n", HexStr(pubKey).c_str());
        printf("    * address (base58): %s\n", CBitcoinAddress(CTxDestination(pubKey.GetID())).ToString().c_str());
}
#endif


BOOST_AUTO_TEST_SUITE(key_tests)

BOOST_AUTO_TEST_CASE(key_test1)
{
    CBitcoinSecret bsecret1, bsecret2, bsecret1C, bsecret2C, baddress1;
    BOOST_CHECK( bsecret1.SetString (strSecret1));
    BOOST_CHECK( bsecret2.SetString (strSecret2));
    BOOST_CHECK( bsecret1C.SetString(strSecret1C));
    BOOST_CHECK( bsecret2C.SetString(strSecret2C));
    BOOST_CHECK(!baddress1.SetString(strAddressBad));

    CKey key1  = bsecret1.GetKey();
    BOOST_CHECK(key1.IsCompressed() == false);
    CKey key2  = bsecret2.GetKey();
    BOOST_CHECK(key2.IsCompressed() == false);
    CKey key1C = bsecret1C.GetKey();
    BOOST_CHECK(key1C.IsCompressed() == true);
    CKey key2C = bsecret2C.GetKey();
    BOOST_CHECK(key2C.IsCompressed() == true);

    CPubKey pubkey1  = key1. GetPubKey();
    CPubKey pubkey2  = key2. GetPubKey();
    CPubKey pubkey1C = key1C.GetPubKey();
    CPubKey pubkey2C = key2C.GetPubKey();

    BOOST_CHECK(key1.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key1C.VerifyPubKey(pubkey1));
    BOOST_CHECK(key1C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2.VerifyPubKey(pubkey1C));
    BOOST_CHECK(key2.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key2.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key2C.VerifyPubKey(pubkey2));
    BOOST_CHECK(key2C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(addr1.Get()  == CTxDestination(pubkey1.GetID()));
    BOOST_CHECK(addr2.Get()  == CTxDestination(pubkey2.GetID()));
    BOOST_CHECK(addr1C.Get() == CTxDestination(pubkey1C.GetID()));
    BOOST_CHECK(addr2C.Get() == CTxDestination(pubkey2C.GetID()));

    for (int n=0; n<16; n++)
    {
        string strMsg = strprintf("Very secret message %i: 11", n);
        uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());

        // normal signatures

        vector<unsigned char> sign1, sign2, sign1C, sign2C;

        BOOST_CHECK(key1.Sign (hashMsg, sign1));
        BOOST_CHECK(key2.Sign (hashMsg, sign2));
        BOOST_CHECK(key1C.Sign(hashMsg, sign1C));
        BOOST_CHECK(key2C.Sign(hashMsg, sign2C));

        BOOST_CHECK( pubkey1.Verify(hashMsg, sign1));
        BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2));
        BOOST_CHECK( pubkey1.Verify(hashMsg, sign1C));
        BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2C));

        BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1));
        BOOST_CHECK( pubkey2.Verify(hashMsg, sign2));
        BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1C));
        BOOST_CHECK( pubkey2.Verify(hashMsg, sign2C));

        BOOST_CHECK( pubkey1C.Verify(hashMsg, sign1));
        BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2));
        BOOST_CHECK( pubkey1C.Verify(hashMsg, sign1C));
        BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2C));

        BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1));
        BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2));
        BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1C));
        BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2C));

        // compact signatures (with key recovery)

        vector<unsigned char> csign1, csign2, csign1C, csign2C;

        BOOST_CHECK(key1.SignCompact (hashMsg, csign1));
        BOOST_CHECK(key2.SignCompact (hashMsg, csign2));
        BOOST_CHECK(key1C.SignCompact(hashMsg, csign1C));
        BOOST_CHECK(key2C.SignCompact(hashMsg, csign2C));

        CPubKey rkey1, rkey2, rkey1C, rkey2C;

        BOOST_CHECK(rkey1.RecoverCompact (hashMsg, csign1));
        BOOST_CHECK(rkey2.RecoverCompact (hashMsg, csign2));
        BOOST_CHECK(rkey1C.RecoverCompact(hashMsg, csign1C));
        BOOST_CHECK(rkey2C.RecoverCompact(hashMsg, csign2C));

        BOOST_CHECK(rkey1  == pubkey1);
        BOOST_CHECK(rkey2  == pubkey2);
        BOOST_CHECK(rkey1C == pubkey1C);
        BOOST_CHECK(rkey2C == pubkey2C);
    }

    // test deterministic signing

    std::vector<unsigned char> detsig, detsigc;
    string strMsg = "Very deterministic message";
    uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());
    BOOST_CHECK(key1.Sign(hashMsg, detsig));
    BOOST_CHECK(key1C.Sign(hashMsg, detsigc));
    BOOST_CHECK(detsig == detsigc);
    BOOST_CHECK(detsig == ParseHex("04fe353e59a1167e47c784ba04e7cfce902694581dfe3e034f28d0658a4852885b871b4c79f82aef8a737d1d0778f6e7ba13237a2a7eb3c71017c2f97660ad155f"));
    BOOST_CHECK(key2.Sign(hashMsg, detsig));
    BOOST_CHECK(key2C.Sign(hashMsg, detsigc));
    BOOST_CHECK(detsig == detsigc);
    BOOST_CHECK(detsig == ParseHex("04000a21ae19f0efc2ecf833b0dd1898ae6efc8b5864ccff9846a8dc77ad8c6bd5f0576d0a84be6b1bb819d526238dd68597bf26a0f1372d1a4fe4fd71de63102d"));
    BOOST_CHECK(key1.SignCompact(hashMsg, detsig));
    BOOST_CHECK(key1C.SignCompact(hashMsg, detsigc));
    BOOST_CHECK(detsig == ParseHex("04fe353e59a1167e47c784ba04e7cfce902694581dfe3e034f28d0658a4852885b871b4c79f82aef8a737d1d0778f6e7ba13237a2a7eb3c71017c2f97660ad155f"));
    BOOST_CHECK(detsigc == ParseHex("04eec159d6cb065a7fe0e3f46cccbda12f95cac0798272f0ee0cc832cffd26ae558a24a5291c13e0a60fb3f1b6a2c184b11edb7ea277c913ebefaf746568710a14"));
    BOOST_CHECK(key2.SignCompact(hashMsg, detsig));
    BOOST_CHECK(key2C.SignCompact(hashMsg, detsigc));
    BOOST_CHECK(detsig == ParseHex("04000a21ae19f0efc2ecf833b0dd1898ae6efc8b5864ccff9846a8dc77ad8c6bd5f0576d0a84be6b1bb819d526238dd68597bf26a0f1372d1a4fe4fd71de63102d"));
    BOOST_CHECK(detsigc == ParseHex("043f0cf987cbf141ae78fb0f465872cbc18556a5328fd6045531aa008bea87607555be20f7aae553b8223567f0e61202a389370942c3b33a35176e2631d64be570"));
}

BOOST_AUTO_TEST_SUITE_END()
