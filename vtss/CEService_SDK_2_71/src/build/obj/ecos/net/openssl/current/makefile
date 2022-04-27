# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/obj/ecos/install
export COMMAND_PREFIX := mipsisa32-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/../eCos/packages
PACKAGE := net/openssl/current
OBJECT_PREFIX := net_openssl
CFLAGS := -Wall -Wpointer-arith -Wundef -Woverloaded-virtual -Wno-write-strings -EL -mips32 -msoft-float -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -G0 -D_KERNEL -D__ECOS -I$(PREFIX)/include/openssl
LDFLAGS := -EL -mips32 -msoft-float -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS :=  tests/randtest.c tests/exptest.c  tests/bntest.c tests/hmactest.c  tests/ideatest.c tests/rc5test.c tests/rsa_test.c tests/md2test.c tests/md4test.c tests/md5test.c tests/shatest.c tests/sha1test.c tests/rmdtest.c tests/despwdtest.c  tests/destest.c tests/rc4test.c tests/rc2test.c tests/bftest.c tests/casttest.c tests/mdc2test.c tests/dsatest.c tests/dhtest.c

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/crypto/asn1/a_object.c src/crypto/asn1/a_bitstr.c src/crypto/asn1/a_utctm.c src/crypto/asn1/a_gentm.c src/crypto/asn1/a_time.c src/crypto/asn1/a_int.c src/crypto/asn1/a_octet.c src/crypto/asn1/a_print.c src/crypto/asn1/a_type.c src/crypto/asn1/a_set.c src/crypto/asn1/a_dup.c src/crypto/asn1/a_d2i_fp.c src/crypto/asn1/a_i2d_fp.c src/crypto/asn1/a_enum.c src/crypto/asn1/a_utf8.c src/crypto/asn1/a_sign.c src/crypto/asn1/a_digest.c src/crypto/asn1/a_verify.c src/crypto/asn1/a_mbstr.c src/crypto/asn1/a_strex.c src/crypto/asn1/x_algor.c src/crypto/asn1/x_val.c src/crypto/asn1/x_pubkey.c src/crypto/asn1/x_sig.c src/crypto/asn1/x_req.c src/crypto/asn1/x_attrib.c src/crypto/asn1/x_name.c src/crypto/asn1/x_x509.c src/crypto/asn1/x_x509a.c src/crypto/asn1/x_crl.c src/crypto/asn1/x_info.c src/crypto/asn1/x_spki.c src/crypto/asn1/nsseq.c src/crypto/asn1/d2i_pu.c src/crypto/asn1/d2i_pr.c src/crypto/asn1/i2d_pu.c src/crypto/asn1/i2d_pr.c src/crypto/asn1/t_req.c src/crypto/asn1/t_x509.c src/crypto/asn1/t_x509a.c src/crypto/asn1/t_crl.c src/crypto/asn1/t_pkey.c src/crypto/asn1/t_spki.c src/crypto/asn1/t_bitst.c src/crypto/asn1/f_int.c src/crypto/asn1/f_string.c src/crypto/asn1/n_pkey.c src/crypto/asn1/f_enum.c src/crypto/asn1/a_hdr.c src/crypto/asn1/x_pkey.c src/crypto/asn1/a_bool.c src/crypto/asn1/x_exten.c src/crypto/asn1/x_bignum.c src/crypto/asn1/x_long.c src/crypto/asn1/asn1_par.c src/crypto/asn1/asn1_lib.c src/crypto/asn1/asn1_err.c src/crypto/asn1/asn1_gen.c src/crypto/asn1/a_meth.c src/crypto/asn1/a_bytes.c src/crypto/asn1/a_strnid.c src/crypto/asn1/evp_asn1.c src/crypto/asn1/asn_pack.c src/crypto/asn1/p5_pbe.c src/crypto/asn1/p5_pbev2.c src/crypto/asn1/p8_pkey.c src/crypto/asn1/tasn_dec.c src/crypto/asn1/tasn_fre.c src/crypto/asn1/tasn_typ.c src/crypto/asn1/tasn_utl.c src/crypto/asn1/tasn_new.c src/crypto/asn1/tasn_enc.c src/crypto/bn/bn_add.c src/crypto/bn/bn_div.c src/crypto/bn/bn_exp.c src/crypto/bn/bn_lib.c src/crypto/bn/bn_ctx.c src/crypto/bn/bn_mul.c src/crypto/bn/bn_mod.c src/crypto/bn/bn_print.c src/crypto/bn/bn_rand.c src/crypto/bn/bn_shift.c src/crypto/bn/bn_word.c src/crypto/bn/bn_blind.c src/crypto/bn/bn_gcd.c src/crypto/bn/bn_prime.c src/crypto/bn/bn_err.c src/crypto/bn/bn_sqr.c src/crypto/bn/bn_asm.c src/crypto/bn/bn_recp.c src/crypto/bn/bn_mont.c src/crypto/bn/bn_mpi.c src/crypto/bn/bn_exp2.c src/crypto/bn/bn_gf2m.c src/crypto/bn/bn_kron.c src/crypto/bn/bn_sqrt.c src/crypto/bn/bn_nist.c src/crypto/dso/dso_err.c src/crypto/evp/encode.c src/crypto/evp/digest.c src/crypto/evp/evp_enc.c src/crypto/evp/evp_key.c src/crypto/evp/e_des.c src/crypto/evp/e_bf.c src/crypto/evp/e_idea.c src/crypto/evp/e_des3.c src/crypto/evp/e_rc4.c src/crypto/evp/e_aes.c src/crypto/evp/names.c src/crypto/evp/e_xcbc_d.c src/crypto/evp/e_rc2.c src/crypto/evp/e_cast.c src/crypto/evp/e_rc5.c src/crypto/evp/m_null.c src/crypto/evp/m_md2.c src/crypto/evp/m_md4.c src/crypto/evp/m_md5.c src/crypto/evp/m_sha.c src/crypto/evp/m_sha1.c src/crypto/evp/m_dss.c src/crypto/evp/m_dss1.c src/crypto/evp/m_mdc2.c src/crypto/evp/m_ripemd.c src/crypto/evp/p_open.c src/crypto/evp/p_seal.c src/crypto/evp/p_sign.c src/crypto/evp/p_verify.c src/crypto/evp/p_lib.c src/crypto/evp/p_enc.c src/crypto/evp/p_dec.c src/crypto/evp/bio_md.c src/crypto/evp/bio_b64.c src/crypto/evp/bio_enc.c src/crypto/evp/evp_err.c src/crypto/evp/e_null.c src/crypto/evp/c_all.c src/crypto/evp/c_allc.c src/crypto/evp/c_alld.c src/crypto/evp/evp_lib.c src/crypto/evp/bio_ok.c src/crypto/evp/evp_pkey.c src/crypto/evp/evp_pbe.c src/crypto/evp/p5_crpt.c src/crypto/evp/p5_crpt2.c src/crypto/evp/enc_min.c src/crypto/evp/m_ecdsa.c src/crypto/evp/e_seed.c src/crypto/ec/ec_err.c src/crypto/ec/ec_key.c src/crypto/ec/ec_lib.c src/crypto/ec/ec_asn1.c src/crypto/ec/ec_curve.c src/crypto/ec/ec_cvt.c src/crypto/ec/ec2_smpl.c src/crypto/ec/ec_mult.c src/crypto/ec/ecp_mont.c src/crypto/ec/ecp_smpl.c src/crypto/ec/ec2_mult.c src/crypto/ec/ecp_nist.c src/crypto/ec/ec_print.c src/crypto/hmac/hmac.c src/crypto/lhash/lhash.c src/crypto/lhash/lh_stats.c src/crypto/objects/o_names.c src/crypto/objects/obj_dat.c src/crypto/objects/obj_lib.c src/crypto/objects/obj_err.c src/crypto/pem/pem_sign.c src/crypto/pem/pem_seal.c src/crypto/pem/pem_info.c src/crypto/pem/pem_lib.c src/crypto/pem/pem_all.c src/crypto/pem/pem_err.c src/crypto/pem/pem_oth.c src/crypto/pem/pem_xaux.c src/crypto/pem/pem_x509.c src/crypto/pem/pem_pkey.c src/crypto/pkcs7/pk7_lib.c src/crypto/pkcs7/pkcs7err.c src/crypto/pkcs7/pk7_doit.c src/crypto/pkcs7/pk7_smime.c src/crypto/pkcs7/pk7_attr.c src/crypto/pkcs7/pk7_mime.c src/crypto/pkcs12/p12_add.c src/crypto/pkcs12/p12_attr.c src/crypto/pkcs12/p12_crpt.c src/crypto/pkcs12/p12_crt.c src/crypto/pkcs12/p12_decr.c src/crypto/pkcs12/p12_init.c src/crypto/pkcs12/p12_key.c src/crypto/pkcs12/p12_kiss.c src/crypto/pkcs12/p12_mutl.c src/crypto/pkcs12/p12_utl.c src/crypto/pkcs12/p12_npas.c src/crypto/pkcs12/p12_p8d.c src/crypto/pkcs12/pk12err.c src/crypto/ocsp/ocsp_asn.c src/crypto/ocsp/ocsp_err.c src/crypto/rand/md_rand.c src/crypto/rand/randfile.c src/crypto/rand/rand_lib.c src/crypto/rand/rand_err.c src/crypto/rand/rand_egd.c src/crypto/rand/rand_ecos.c src/crypto/stack/stack.c src/crypto/txt_db/txt_db.c src/crypto/x509/x509_def.c src/crypto/x509/x509_d2.c src/crypto/x509/x509_r2x.c src/crypto/x509/x509_cmp.c src/crypto/x509/x509_obj.c src/crypto/x509/x509_req.c src/crypto/x509/x509spki.c src/crypto/x509/x509_vfy.c src/crypto/x509/x509_set.c src/crypto/x509/x509rset.c src/crypto/x509/x509_err.c src/crypto/x509/x509name.c src/crypto/x509/x509_v3.c src/crypto/x509/x509_ext.c src/crypto/x509/x509_att.c src/crypto/x509/x509type.c src/crypto/x509/x509_lu.c src/crypto/x509/x_all.c src/crypto/x509/x509_txt.c src/crypto/x509/x509_trs.c src/crypto/x509/by_file.c src/crypto/x509/by_dir.c src/crypto/x509/x509_vpm.c src/crypto/x509v3/v3_bcons.c src/crypto/x509v3/v3_bitst.c src/crypto/x509v3/v3_conf.c src/crypto/x509v3/v3_extku.c src/crypto/x509v3/v3_ia5.c src/crypto/x509v3/v3_lib.c src/crypto/x509v3/v3_prn.c src/crypto/x509v3/v3_utl.c src/crypto/x509v3/v3err.c src/crypto/x509v3/v3_genn.c src/crypto/x509v3/v3_alt.c src/crypto/x509v3/v3_skey.c src/crypto/x509v3/v3_akey.c src/crypto/x509v3/v3_pku.c src/crypto/x509v3/v3_int.c src/crypto/x509v3/v3_enum.c src/crypto/x509v3/v3_sxnet.c src/crypto/x509v3/v3_cpols.c src/crypto/x509v3/v3_crld.c src/crypto/x509v3/v3_purp.c src/crypto/x509v3/v3_info.c src/crypto/x509v3/v3_ocsp.c src/crypto/x509v3/v3_pcia.c src/crypto/x509v3/v3_akeya.c src/crypto/x509v3/v3_pci.c src/crypto/x509v3/v3_pcons.c src/crypto/x509v3/pcy_cache.c src/crypto/x509v3/pcy_data.c src/crypto/x509v3/v3_ncons.c src/crypto/x509v3/v3_pmaps.c src/crypto/x509v3/v3_asid.c src/crypto/x509v3/v3_addr.c src/crypto/x509v3/pcy_tree.c src/crypto/x509v3/pcy_lib.c src/crypto/x509v3/pcy_node.c src/crypto/x509v3/pcy_map.c src/crypto/conf/conf_err.c src/crypto/conf/conf_lib.c src/crypto/conf/conf_api.c src/crypto/conf/conf_def.c src/crypto/conf/conf_mod.c src/crypto/engine/eng_init.c src/crypto/engine/tb_dsa.c src/crypto/engine/tb_rsa.c src/crypto/engine/tb_digest.c src/crypto/engine/tb_rand.c src/crypto/engine/eng_table.c src/crypto/engine/eng_err.c src/crypto/engine/tb_dh.c src/crypto/engine/tb_cipher.c src/crypto/engine/eng_lib.c src/crypto/engine/eng_pkey.c src/crypto/engine/tb_ecdsa.c src/crypto/engine/tb_ecdh.c src/crypto/err/err_all.c src/crypto/err/err_prn.c src/crypto/err/err.c src/crypto/err/err_def.c src/crypto/err/err_str.c src/crypto/ui/ui_lib.c src/crypto/ui/ui_err.c src/crypto/ui/ui_openssl.c src/crypto/cryptlib.c src/crypto/mem.c src/crypto/mem_clr.c src/crypto/mem_dbg.c src/crypto/cversion.c src/crypto/ex_data.c src/crypto/cpt_err.c src/crypto/ebcdic.c src/crypto/o_time.c src/crypto/uid.c src/crypto/seed/seed.c src/crypto/seed/seed_cbc.c src/crypto/seed/seed_cfb.c src/crypto/seed/seed_ecb.c src/crypto/seed/seed_ofb.c src/extra/ssl_no_fp.c src/crypto/aes/aes_cbc.c src/crypto/aes/aes_cfb.c src/crypto/aes/aes_ecb.c src/crypto/aes/aes_ofb.c src/crypto/aes/aes_core.c src/crypto/idea/i_cbc.c src/crypto/idea/i_cfb64.c src/crypto/idea/i_ofb64.c src/crypto/idea/i_ecb.c src/crypto/idea/i_skey.c src/crypto/rc5/rc5_skey.c src/crypto/rc5/rc5_ecb.c src/crypto/rc5/rc5_enc.c src/crypto/rc5/rc5cfb64.c src/crypto/rc5/rc5ofb64.c src/crypto/rsa/rsa_eay.c src/crypto/rsa/rsa_chk.c src/crypto/rsa/rsa_depr.c src/crypto/rsa/rsa_gen.c src/crypto/rsa/rsa_lib.c src/crypto/rsa/rsa_sign.c src/crypto/rsa/rsa_saos.c src/crypto/rsa/rsa_err.c src/crypto/rsa/rsa_pk1.c src/crypto/rsa/rsa_ssl.c src/crypto/rsa/rsa_none.c src/crypto/rsa/rsa_oaep.c src/crypto/rsa/rsa_null.c src/crypto/rsa/rsa_asn1.c src/crypto/rsa/rsa_eng.c src/crypto/rsa/rsa_x931.c src/crypto/md2/md2_dgst.c src/crypto/md2/md2_one.c src/crypto/md4/md4_dgst.c src/crypto/md4/md4_one.c src/crypto/md5/md5_dgst.c src/crypto/md5/md5_one.c src/crypto/sha/sha_dgst.c src/crypto/sha/sha_one.c src/crypto/sha/sha256.c src/crypto/sha/sha512.c src/crypto/sha/sha1dgst.c src/crypto/sha/sha1_one.c src/crypto/ripemd/rmd_dgst.c src/crypto/ripemd/rmd_one.c src/crypto/des/set_key.c src/crypto/des/ecb_enc.c src/crypto/des/cbc_enc.c src/crypto/des/ecb3_enc.c src/crypto/des/cfb64enc.c src/crypto/des/cfb64ede.c src/crypto/des/cfb_enc.c src/crypto/des/ofb64ede.c src/crypto/des/enc_read.c src/crypto/des/enc_writ.c src/crypto/des/ofb64enc.c src/crypto/des/ofb_enc.c src/crypto/des/str2key.c src/crypto/des/pcbc_enc.c src/crypto/des/qud_cksm.c src/crypto/des/rand_key.c src/crypto/des/des_enc.c src/crypto/des/des_old.c src/crypto/des/fcrypt.c src/crypto/des/fcrypt_b.c src/crypto/des/xcbc_enc.c src/crypto/des/rpc_enc.c src/crypto/des/cbc_cksm.c src/crypto/des/ede_cbcm_enc.c src/crypto/des/read2pwd.c src/crypto/des/read_pwd.c src/crypto/rc4/rc4_skey.c src/crypto/rc4/rc4_enc.c src/crypto/rc2/rc2_ecb.c src/crypto/rc2/rc2_skey.c src/crypto/rc2/rc2_cbc.c src/crypto/rc2/rc2cfb64.c src/crypto/rc2/rc2ofb64.c src/crypto/bf/bf_skey.c src/crypto/bf/bf_ecb.c src/crypto/bf/bf_enc.c src/crypto/bf/bf_cfb64.c src/crypto/bf/bf_ofb64.c src/crypto/cast/c_skey.c src/crypto/cast/c_ecb.c src/crypto/cast/c_cfb64.c src/crypto/cast/c_ofb64.c src/crypto/cast/c_enc.c src/crypto/mdc2/mdc2dgst.c src/crypto/mdc2/mdc2_one.c src/crypto/dsa/dsa_gen.c src/crypto/dsa/dsa_key.c src/crypto/dsa/dsa_lib.c src/crypto/dsa/dsa_asn1.c src/crypto/dsa/dsa_vrf.c src/crypto/dsa/dsa_sign.c src/crypto/dsa/dsa_err.c src/crypto/dsa/dsa_ossl.c src/crypto/dsa/dsa_utl.c src/crypto/dh/dh_gen.c src/crypto/dh/dh_key.c src/crypto/dh/dh_lib.c src/crypto/dh/dh_check.c src/crypto/dh/dh_err.c src/crypto/dh/dh_asn1.c src/crypto/bio/bio_lib.c src/crypto/bio/bio_cb.c src/crypto/bio/bio_err.c src/crypto/bio/bss_mem.c src/crypto/bio/bss_null.c src/crypto/bio/bss_fd.c src/crypto/bio/bss_file.c src/crypto/bio/bss_sock.c src/crypto/bio/bss_conn.c src/crypto/bio/bf_null.c src/crypto/bio/bf_buff.c src/crypto/bio/b_print.c src/crypto/bio/b_dump.c src/crypto/bio/b_sock.c src/crypto/bio/bss_acpt.c src/crypto/bio/bf_nbio.c src/crypto/bio/bss_log.c src/crypto/bio/bss_bio.c src/crypto/buffer/buffer.c src/crypto/buffer/buf_err.c src/crypto/buffer/buf_str.c src/crypto/comp/comp_lib.c src/crypto/comp/c_rle.c src/crypto/comp/c_zlib.c src/crypto/comp/comp_err.c src/ssl/bio_ssl.c src/ssl/s23_pkt.c src/ssl/s2_lib.c src/ssl/s3_both.c src/ssl/s3_cbc.c src/ssl/s3_meth.c src/ssl/ssl_asn1.c src/ssl/ssl_err2.c src/ssl/ssl_stat.c src/ssl/t1_lib.c src/ssl/s23_clnt.c src/ssl/s23_srvr.c src/ssl/s2_meth.c src/ssl/s3_clnt.c src/ssl/s3_pkt.c src/ssl/ssl_cert.c src/ssl/ssl_lib.c src/ssl/ssl_txt.c src/ssl/t1_meth.c src/ssl/s23_lib.c src/ssl/s2_clnt.c src/ssl/s2_pkt.c src/ssl/s3_enc.c src/ssl/s3_srvr.c src/ssl/ssl_ciph.c src/ssl/ssl_rsa.c src/ssl/t1_clnt.c src/ssl/t1_srvr.c src/ssl/s23_meth.c src/ssl/s2_enc.c src/ssl/s2_srvr.c src/ssl/s3_lib.c src/ssl/ssl_algs.c src/ssl/ssl_err.c src/ssl/ssl_sess.c src/ssl/t1_enc.c src/ssl/t1_reneg.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/openssl/md32_common.h $(PREFIX)/include/openssl/ocsp.h $(PREFIX)/include/openssl/rsa.h $(PREFIX)/include/openssl/x509.h $(PREFIX)/include/openssl/des_locl.h $(PREFIX)/include/openssl/seed_locl.h $(PREFIX)/include/openssl/safestack.h $(PREFIX)/include/openssl/dtls1.h $(PREFIX)/include/openssl/crypto.h $(PREFIX)/include/openssl/ext_dat.h $(PREFIX)/include/openssl/des_old.h $(PREFIX)/include/openssl/camellia.h $(PREFIX)/include/openssl/evp.h $(PREFIX)/include/openssl/o_dir.h $(PREFIX)/include/openssl/bn_prime.h $(PREFIX)/include/openssl/ui_locl.h $(PREFIX)/include/openssl/jpake.h $(PREFIX)/include/openssl/kssl.h $(PREFIX)/include/openssl/pq_compat.h $(PREFIX)/include/openssl/kssl_lcl.h $(PREFIX)/include/openssl/ui.h $(PREFIX)/include/openssl/bf_pi.h $(PREFIX)/include/openssl/cms.h $(PREFIX)/include/openssl/md5.h $(PREFIX)/include/openssl/asn1_mac.h $(PREFIX)/include/openssl/e_os2.h $(PREFIX)/include/openssl/blowfish.h $(PREFIX)/include/openssl/rc2.h $(PREFIX)/include/openssl/ech_locl.h $(PREFIX)/include/openssl/rmd_locl.h $(PREFIX)/include/openssl/cmll_locl.h $(PREFIX)/include/openssl/seed.h $(PREFIX)/include/openssl/err.h $(PREFIX)/include/openssl/ssl23.h $(PREFIX)/include/openssl/txt_db.h $(PREFIX)/include/openssl/dh.h $(PREFIX)/include/openssl/spr.h $(PREFIX)/include/openssl/ssl_locl.h $(PREFIX)/include/openssl/aes_locl.h $(PREFIX)/include/openssl/cast.h $(PREFIX)/include/openssl/fips_rand.h $(PREFIX)/include/openssl/charmap.h $(PREFIX)/include/openssl/des_ver.h $(PREFIX)/include/openssl/bf_locl.h $(PREFIX)/include/openssl/cast_s.h $(PREFIX)/include/openssl/md5_locl.h $(PREFIX)/include/openssl/conf_api.h $(PREFIX)/include/openssl/idea_lcl.h $(PREFIX)/include/openssl/rc4.h $(PREFIX)/include/openssl/conf.h $(PREFIX)/include/openssl/ec_lcl.h $(PREFIX)/include/openssl/lhash.h $(PREFIX)/include/openssl/opensslv.h $(PREFIX)/include/openssl/dso.h $(PREFIX)/include/openssl/example.h $(PREFIX)/include/openssl/aes.h $(PREFIX)/include/openssl/bn_lcl.h $(PREFIX)/include/openssl/buffer.h $(PREFIX)/include/openssl/idea.h $(PREFIX)/include/openssl/x509_vfy.h $(PREFIX)/include/openssl/pem2.h $(PREFIX)/include/openssl/stack.h $(PREFIX)/include/openssl/cms_lcl.h $(PREFIX)/include/openssl/conf_def.h $(PREFIX)/include/openssl/fips.h $(PREFIX)/include/openssl/rc4_locl.h $(PREFIX)/include/openssl/eng_int.h $(PREFIX)/include/openssl/des.h $(PREFIX)/include/openssl/rand.h $(PREFIX)/include/openssl/ecdh.h $(PREFIX)/include/openssl/ssl2.h $(PREFIX)/include/openssl/md4.h $(PREFIX)/include/openssl/bio_lcl.h $(PREFIX)/include/openssl/ssl.h $(PREFIX)/include/openssl/objects.h $(PREFIX)/include/openssl/x509v3.h $(PREFIX)/include/openssl/o_time.h $(PREFIX)/include/openssl/evp_locl.h $(PREFIX)/include/openssl/ecs_locl.h $(PREFIX)/include/openssl/ossl_typ.h $(PREFIX)/include/openssl/pqueue.h $(PREFIX)/include/openssl/tmdiff.h $(PREFIX)/include/openssl/hmac.h $(PREFIX)/include/openssl/comp.h $(PREFIX)/include/openssl/ssl3.h $(PREFIX)/include/openssl/pkcs12.h $(PREFIX)/include/openssl/asn1t.h $(PREFIX)/include/openssl/str_locl.h $(PREFIX)/include/openssl/obj_mac.h $(PREFIX)/include/openssl/rpc_des.h $(PREFIX)/include/openssl/pkcs7.h $(PREFIX)/include/openssl/bn.h $(PREFIX)/include/openssl/ssl_no_fp.h $(PREFIX)/include/openssl/ec.h $(PREFIX)/include/openssl/cryptlib.h $(PREFIX)/include/openssl/rand_lcl.h $(PREFIX)/include/openssl/asn1.h $(PREFIX)/include/openssl/pem.h $(PREFIX)/include/openssl/mdc2.h $(PREFIX)/include/openssl/rc5.h $(PREFIX)/include/openssl/bio.h $(PREFIX)/include/openssl/pcy_int.h $(PREFIX)/include/openssl/ui_compat.h $(PREFIX)/include/openssl/krb5_asn.h $(PREFIX)/include/openssl/store.h $(PREFIX)/include/openssl/cast_lcl.h $(PREFIX)/include/openssl/md4_locl.h $(PREFIX)/include/openssl/sha.h $(PREFIX)/include/openssl/dsa.h $(PREFIX)/include/openssl/tls1.h $(PREFIX)/include/openssl/rc5_locl.h $(PREFIX)/include/openssl/md2.h $(PREFIX)/include/openssl/ripemd.h $(PREFIX)/include/openssl/engine.h $(PREFIX)/include/openssl/ebcdic.h $(PREFIX)/include/openssl/ecdsa.h $(PREFIX)/include/openssl/sha_locl.h $(PREFIX)/include/openssl/e_os.h $(PREFIX)/include/openssl/symhacks.h $(PREFIX)/include/openssl/rmdconst.h $(PREFIX)/include/openssl/rc2_locl.h $(PREFIX)/include/openssl/obj_dat.h $(PREFIX)/include/openssl/opensslconf.h

$(PREFIX)/include/openssl/md32_common.h: $(REPOSITORY)/$(PACKAGE)/include/md32_common.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ocsp.h: $(REPOSITORY)/$(PACKAGE)/include/ocsp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rsa.h: $(REPOSITORY)/$(PACKAGE)/include/rsa.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/x509.h: $(REPOSITORY)/$(PACKAGE)/include/x509.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/des_locl.h: $(REPOSITORY)/$(PACKAGE)/include/des_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/seed_locl.h: $(REPOSITORY)/$(PACKAGE)/include/seed_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/safestack.h: $(REPOSITORY)/$(PACKAGE)/include/safestack.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/dtls1.h: $(REPOSITORY)/$(PACKAGE)/include/dtls1.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/crypto.h: $(REPOSITORY)/$(PACKAGE)/include/crypto.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ext_dat.h: $(REPOSITORY)/$(PACKAGE)/include/ext_dat.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/des_old.h: $(REPOSITORY)/$(PACKAGE)/include/des_old.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/camellia.h: $(REPOSITORY)/$(PACKAGE)/include/camellia.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/evp.h: $(REPOSITORY)/$(PACKAGE)/include/evp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/o_dir.h: $(REPOSITORY)/$(PACKAGE)/include/o_dir.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/bn_prime.h: $(REPOSITORY)/$(PACKAGE)/include/bn_prime.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ui_locl.h: $(REPOSITORY)/$(PACKAGE)/include/ui_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/jpake.h: $(REPOSITORY)/$(PACKAGE)/include/jpake.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/kssl.h: $(REPOSITORY)/$(PACKAGE)/include/kssl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/pq_compat.h: $(REPOSITORY)/$(PACKAGE)/include/pq_compat.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/kssl_lcl.h: $(REPOSITORY)/$(PACKAGE)/include/kssl_lcl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ui.h: $(REPOSITORY)/$(PACKAGE)/include/ui.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/bf_pi.h: $(REPOSITORY)/$(PACKAGE)/include/bf_pi.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/cms.h: $(REPOSITORY)/$(PACKAGE)/include/cms.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/md5.h: $(REPOSITORY)/$(PACKAGE)/include/md5.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/asn1_mac.h: $(REPOSITORY)/$(PACKAGE)/include/asn1_mac.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/e_os2.h: $(REPOSITORY)/$(PACKAGE)/include/e_os2.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/blowfish.h: $(REPOSITORY)/$(PACKAGE)/include/blowfish.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rc2.h: $(REPOSITORY)/$(PACKAGE)/include/rc2.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ech_locl.h: $(REPOSITORY)/$(PACKAGE)/include/ech_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rmd_locl.h: $(REPOSITORY)/$(PACKAGE)/include/rmd_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/cmll_locl.h: $(REPOSITORY)/$(PACKAGE)/include/cmll_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/seed.h: $(REPOSITORY)/$(PACKAGE)/include/seed.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/err.h: $(REPOSITORY)/$(PACKAGE)/include/err.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ssl23.h: $(REPOSITORY)/$(PACKAGE)/include/ssl23.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/txt_db.h: $(REPOSITORY)/$(PACKAGE)/include/txt_db.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/dh.h: $(REPOSITORY)/$(PACKAGE)/include/dh.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/spr.h: $(REPOSITORY)/$(PACKAGE)/include/spr.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ssl_locl.h: $(REPOSITORY)/$(PACKAGE)/include/ssl_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/aes_locl.h: $(REPOSITORY)/$(PACKAGE)/include/aes_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/cast.h: $(REPOSITORY)/$(PACKAGE)/include/cast.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/fips_rand.h: $(REPOSITORY)/$(PACKAGE)/include/fips_rand.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/charmap.h: $(REPOSITORY)/$(PACKAGE)/include/charmap.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/des_ver.h: $(REPOSITORY)/$(PACKAGE)/include/des_ver.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/bf_locl.h: $(REPOSITORY)/$(PACKAGE)/include/bf_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/cast_s.h: $(REPOSITORY)/$(PACKAGE)/include/cast_s.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/md5_locl.h: $(REPOSITORY)/$(PACKAGE)/include/md5_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/conf_api.h: $(REPOSITORY)/$(PACKAGE)/include/conf_api.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/idea_lcl.h: $(REPOSITORY)/$(PACKAGE)/include/idea_lcl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rc4.h: $(REPOSITORY)/$(PACKAGE)/include/rc4.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/conf.h: $(REPOSITORY)/$(PACKAGE)/include/conf.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ec_lcl.h: $(REPOSITORY)/$(PACKAGE)/include/ec_lcl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/lhash.h: $(REPOSITORY)/$(PACKAGE)/include/lhash.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/opensslv.h: $(REPOSITORY)/$(PACKAGE)/include/opensslv.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/dso.h: $(REPOSITORY)/$(PACKAGE)/include/dso.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/example.h: $(REPOSITORY)/$(PACKAGE)/include/example.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/aes.h: $(REPOSITORY)/$(PACKAGE)/include/aes.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/bn_lcl.h: $(REPOSITORY)/$(PACKAGE)/include/bn_lcl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/buffer.h: $(REPOSITORY)/$(PACKAGE)/include/buffer.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/idea.h: $(REPOSITORY)/$(PACKAGE)/include/idea.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/x509_vfy.h: $(REPOSITORY)/$(PACKAGE)/include/x509_vfy.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/pem2.h: $(REPOSITORY)/$(PACKAGE)/include/pem2.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/stack.h: $(REPOSITORY)/$(PACKAGE)/include/stack.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/cms_lcl.h: $(REPOSITORY)/$(PACKAGE)/include/cms_lcl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/conf_def.h: $(REPOSITORY)/$(PACKAGE)/include/conf_def.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/fips.h: $(REPOSITORY)/$(PACKAGE)/include/fips.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rc4_locl.h: $(REPOSITORY)/$(PACKAGE)/include/rc4_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/eng_int.h: $(REPOSITORY)/$(PACKAGE)/include/eng_int.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/des.h: $(REPOSITORY)/$(PACKAGE)/include/des.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rand.h: $(REPOSITORY)/$(PACKAGE)/include/rand.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ecdh.h: $(REPOSITORY)/$(PACKAGE)/include/ecdh.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ssl2.h: $(REPOSITORY)/$(PACKAGE)/include/ssl2.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/md4.h: $(REPOSITORY)/$(PACKAGE)/include/md4.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/bio_lcl.h: $(REPOSITORY)/$(PACKAGE)/include/bio_lcl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ssl.h: $(REPOSITORY)/$(PACKAGE)/include/ssl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/objects.h: $(REPOSITORY)/$(PACKAGE)/include/objects.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/x509v3.h: $(REPOSITORY)/$(PACKAGE)/include/x509v3.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/o_time.h: $(REPOSITORY)/$(PACKAGE)/include/o_time.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/evp_locl.h: $(REPOSITORY)/$(PACKAGE)/include/evp_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ecs_locl.h: $(REPOSITORY)/$(PACKAGE)/include/ecs_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ossl_typ.h: $(REPOSITORY)/$(PACKAGE)/include/ossl_typ.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/pqueue.h: $(REPOSITORY)/$(PACKAGE)/include/pqueue.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/tmdiff.h: $(REPOSITORY)/$(PACKAGE)/include/tmdiff.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/hmac.h: $(REPOSITORY)/$(PACKAGE)/include/hmac.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/comp.h: $(REPOSITORY)/$(PACKAGE)/include/comp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ssl3.h: $(REPOSITORY)/$(PACKAGE)/include/ssl3.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/pkcs12.h: $(REPOSITORY)/$(PACKAGE)/include/pkcs12.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/asn1t.h: $(REPOSITORY)/$(PACKAGE)/include/asn1t.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/str_locl.h: $(REPOSITORY)/$(PACKAGE)/include/str_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/obj_mac.h: $(REPOSITORY)/$(PACKAGE)/include/obj_mac.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rpc_des.h: $(REPOSITORY)/$(PACKAGE)/include/rpc_des.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/pkcs7.h: $(REPOSITORY)/$(PACKAGE)/include/pkcs7.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/bn.h: $(REPOSITORY)/$(PACKAGE)/include/bn.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ssl_no_fp.h: $(REPOSITORY)/$(PACKAGE)/include/ssl_no_fp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ec.h: $(REPOSITORY)/$(PACKAGE)/include/ec.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/cryptlib.h: $(REPOSITORY)/$(PACKAGE)/include/cryptlib.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rand_lcl.h: $(REPOSITORY)/$(PACKAGE)/include/rand_lcl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/asn1.h: $(REPOSITORY)/$(PACKAGE)/include/asn1.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/pem.h: $(REPOSITORY)/$(PACKAGE)/include/pem.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/mdc2.h: $(REPOSITORY)/$(PACKAGE)/include/mdc2.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rc5.h: $(REPOSITORY)/$(PACKAGE)/include/rc5.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/bio.h: $(REPOSITORY)/$(PACKAGE)/include/bio.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/pcy_int.h: $(REPOSITORY)/$(PACKAGE)/include/pcy_int.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ui_compat.h: $(REPOSITORY)/$(PACKAGE)/include/ui_compat.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/krb5_asn.h: $(REPOSITORY)/$(PACKAGE)/include/krb5_asn.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/store.h: $(REPOSITORY)/$(PACKAGE)/include/store.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/cast_lcl.h: $(REPOSITORY)/$(PACKAGE)/include/cast_lcl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/md4_locl.h: $(REPOSITORY)/$(PACKAGE)/include/md4_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/sha.h: $(REPOSITORY)/$(PACKAGE)/include/sha.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/dsa.h: $(REPOSITORY)/$(PACKAGE)/include/dsa.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/tls1.h: $(REPOSITORY)/$(PACKAGE)/include/tls1.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rc5_locl.h: $(REPOSITORY)/$(PACKAGE)/include/rc5_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/md2.h: $(REPOSITORY)/$(PACKAGE)/include/md2.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ripemd.h: $(REPOSITORY)/$(PACKAGE)/include/ripemd.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/engine.h: $(REPOSITORY)/$(PACKAGE)/include/engine.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ebcdic.h: $(REPOSITORY)/$(PACKAGE)/include/ebcdic.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/ecdsa.h: $(REPOSITORY)/$(PACKAGE)/include/ecdsa.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/sha_locl.h: $(REPOSITORY)/$(PACKAGE)/include/sha_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/e_os.h: $(REPOSITORY)/$(PACKAGE)/include/e_os.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/symhacks.h: $(REPOSITORY)/$(PACKAGE)/include/symhacks.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rmdconst.h: $(REPOSITORY)/$(PACKAGE)/include/rmdconst.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/rc2_locl.h: $(REPOSITORY)/$(PACKAGE)/include/rc2_locl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/obj_dat.h: $(REPOSITORY)/$(PACKAGE)/include/obj_dat.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/openssl/opensslconf.h: $(REPOSITORY)/$(PACKAGE)/include/opensslconf.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

