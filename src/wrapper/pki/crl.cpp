#include "../stdafx.h"

#include "crl.h"

void CRL::read(Handle<Bio> in, DataFormat::DATA_FORMAT format) {
	LOGGER_FN();

	try{
		if (in.isEmpty()){
			THROW_EXCEPTION(0, CRL, NULL, ERROR_PARAMETER_NULL, 1);
		}
			
		X509_CRL *crl = NULL;

		switch (format){
		case DataFormat::BASE64:
			LOGGER_OPENSSL("PEM_read_bio_X509_CRL");
			crl = PEM_read_bio_X509_CRL(in->internal(), NULL, NULL, NULL);
			if (!crl){
				THROW_EXCEPTION(0, CRL, NULL, ERROR_CRL_BAD_PEM_INPUT_DATA);
			}				
			break;
		case DataFormat::DER:
			LOGGER_OPENSSL("d2i_X509_CRL_bio");
			crl = d2i_X509_CRL_bio(in->internal(), NULL);
			if (!crl){
				THROW_EXCEPTION(0, CRL, NULL, ERROR_CRL_BAD_DIR_INPUT_DATA);
			}				
			break;
		default:
			THROW_EXCEPTION(0, CRL, NULL, ERROR_DATA_FORMAT_UNKNOWN_FORMAT, format);
		}
		this->setData(crl);
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, CRL, e, "Error read CRL");
	}	
}

void CRL::write(Handle<Bio> out, DataFormat::DATA_FORMAT format) {
	LOGGER_FN();

	try{
		if (out.isEmpty()){
			THROW_EXCEPTION(0, CRL, NULL, ERROR_PARAMETER_NULL, 1);
		}
			
		switch (format){
		case DataFormat::DER:
			LOGGER_OPENSSL(i2d_X509_CRL_bio);
			if (!i2d_X509_CRL_bio(out->internal(), this->internal())){
				THROW_OPENSSL_EXCEPTION(0, CRL, NULL, "i2d_X509_CRL_bio");
			}				
			break;
		case DataFormat::BASE64:
			LOGGER_OPENSSL(PEM_write_bio_X509_CRL);
			if (!PEM_write_bio_X509_CRL(out->internal(), this->internal())){
				THROW_OPENSSL_EXCEPTION(0, CRL, NULL, "PEM_write_bio_X509_CRL");
			}				
			break;
		}
		out->flush();
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, CRL, e, "Error write CRL to file");
	}
	
}

Handle<CRL> CRL::duplicate(){
	LOGGER_FN();
	try{
		X509_CRL *crl = NULL;
		LOGGER_OPENSSL(X509_CRL_dup);
		crl = X509_CRL_dup(this->internal());
		if (!crl){
			THROW_OPENSSL_EXCEPTION(0, CRL, NULL, "X509_CRL_dup");
		}
		return new CRL(crl);
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, CRL, e, "Eror duplicate CRL");
	}	
}

int CRL::equals(Handle<CRL> crl){
	LOGGER_FN();
	try{
		LOGGER_OPENSSL(X509_CRL_cmp);
		if (X509_CRL_cmp(this->internal(), crl->internal()) == 0){
			return 0;
		}
		else{
			return -1;
		}
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, CRL, e, "Error compare CRL");
	}	
}

int CRL::compare(Handle<CRL> crl){
	LOGGER_FN();

	LOGGER_OPENSSL(X509_CRL_cmp);
	int res = X509_CRL_cmp(this->internal(), crl->internal());

	return res;
}

Handle<std::string> CRL::getThumbprint()
{
	LOGGER_FN();

	return this->hash(EVP_sha1());
}

Handle<std::string> CRL::hash(Handle<std::string> algorithm){
	LOGGER_FN();

	LOGGER_OPENSSL(EVP_get_digestbyname);
	const EVP_MD *md = EVP_get_digestbyname(algorithm->c_str());
	if (!md) {
		THROW_OPENSSL_EXCEPTION(0, CRL, NULL, "EVP_get_digestbyname");
	}

	return this->hash(md);
}

Handle<std::string> CRL::hash(const EVP_MD *md) {
	LOGGER_FN();

	unsigned char hash[EVP_MAX_MD_SIZE] = { 0 };
	unsigned int hashlen = 0;

	LOGGER_OPENSSL(X509_CRL_digest);
	if (!X509_CRL_digest(this->internal(), md, hash, &hashlen)) {
		THROW_OPENSSL_EXCEPTION(0, CRL, NULL, "X509_CRL_digest");
	}

	Handle<std::string> res = new std::string((char *)hash, hashlen);

	return res;
}

long CRL::getVersion()
{
	LOGGER_FN();
	std::string strBuf;

	LOGGER_OPENSSL(X509_CRL_get_version);
	long ver = X509_CRL_get_version(this->internal());

	return ver;
}

Handle<std::string> CRL::getSigAlgName(){
	LOGGER_FN();

	try{
		LOGGER_OPENSSL(OBJ_obj2nid);
		int pkey_nid = OBJ_obj2nid(this->internal()->sig_alg->algorithm);

		if (pkey_nid == NID_undef) {
			THROW_EXCEPTION(0, CRL, NULL, "Can not get key nid");
		}

		LOGGER_OPENSSL(OBJ_nid2ln);
		std::string sslbuf = OBJ_nid2ln(pkey_nid);

		Handle<std::string> res = new std::string(sslbuf.c_str(), sslbuf.length());

		return res;
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, CRL, e, "Error get CRL signature algorithm long name");
	}	
}

Handle<std::string> CRL::getSigAlgShortName(){
	LOGGER_FN();

	try{
		LOGGER_OPENSSL(OBJ_obj2nid);
		int pkey_nid = OBJ_obj2nid(this->internal()->sig_alg->algorithm);
		if (pkey_nid == NID_undef) {
			THROW_EXCEPTION(0, CRL, NULL, "Can not get key nid");
		}

		LOGGER_OPENSSL(OBJ_nid2ln);
		std::string sslbuf = OBJ_nid2sn(pkey_nid);

		Handle<std::string> res = new std::string(sslbuf.c_str(), sslbuf.length());

		return res;
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, CRL, e, "Error get CRL signature algorithm short name");
	}
}

Handle<std::string> CRL::getSigAlgOID(){
	LOGGER_FN();

	try{
		char buf[100];
		LOGGER_OPENSSL(OBJ_obj2txt);
		int bufLen = 0;
		if ((bufLen = OBJ_obj2txt(buf, 100, this->internal()->sig_alg->algorithm, 1)) <= 0){
			THROW_OPENSSL_EXCEPTION(0, Algorithm, NULL, "OBJ_obj2txt");
		}
			
		std::string *res = new std::string(buf, bufLen);

		return res;
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, CRL, e, "Error get CRL signature OID");
	}
}
Handle<std::string> CRL::issuerName()
{
	LOGGER_FN();
	

	LOGGER_OPENSSL(X509_CRL_get_issuer);
	X509_NAME *name = X509_CRL_get_issuer(this->internal());
	if (!name)
		THROW_EXCEPTION(1, CRL, NULL, "X509_NAME is NULL");

	LOGGER_OPENSSL(X509_NAME_oneline);
	char *str_name = X509_NAME_oneline(name, NULL, 0);
	if (!str_name)
		THROW_EXCEPTION(1, CRL, NULL, "X509_NAME_oneline");

	Handle<std::string> res = new std::string(str_name);
	OPENSSL_free(str_name);

	return res;
}

Handle<std::string> CRL::issuerFriendlyName()
{
	LOGGER_FN();

	LOGGER_OPENSSL(X509_get_issuer_name);
	return GetCommonName(X509_CRL_get_issuer(this->internal()));
}

Handle<std::string> CRL::GetCommonName(X509_NAME *a){
	LOGGER_FN();

	Handle<std::string> name = new std::string("");
	if (a == NULL)
		THROW_EXCEPTION(0, Certificate, NULL, "Parameter 1 can not be NULL");

	int nid = NID_commonName;
	LOGGER_OPENSSL(X509_NAME_get_index_by_NID);
	int index = X509_NAME_get_index_by_NID(a, nid, -1);
	if (index >= 0) {
		LOGGER_OPENSSL(X509_NAME_get_entry);
		X509_NAME_ENTRY *issuerNameCommonName = X509_NAME_get_entry(a, index);

		if (issuerNameCommonName) {
			LOGGER_OPENSSL(X509_NAME_ENTRY_get_data);
			ASN1_STRING *issuerCNASN1 = X509_NAME_ENTRY_get_data(issuerNameCommonName);

			if (issuerCNASN1 != NULL) {
				unsigned char *utf = NULL;
				LOGGER_OPENSSL(ASN1_STRING_to_UTF8);
				ASN1_STRING_to_UTF8(&utf, issuerCNASN1);
				name = new std::string((char *)utf);
				OPENSSL_free(utf);
			}
		}
	}
	else {
		return new std::string("No common name");
	}
	return name;
}

Handle<std::string> CRL::getEncoded(){
	LOGGER_FN();

	try{
		LOGGER_OPENSSL(BIO_new);
		BIO * bio_out = BIO_new(BIO_s_mem());
		LOGGER_OPENSSL(i2d_X509_CRL_bio);
		if (!i2d_X509_CRL_bio(bio_out, this->internal())){
			THROW_OPENSSL_EXCEPTION(0, CRL, NULL, "i2d_X509_CRL_bio");
		}
		BUF_MEM *bio_buf;
		LOGGER_OPENSSL(BIO_get_mem_ptr);
		BIO_get_mem_ptr(bio_out, &bio_buf);
		Handle<std::string> res = new std::string(bio_buf->data, bio_buf->length);
		LOGGER_OPENSSL(BIO_free);
		BIO_free(bio_out);

		return res;
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, CRL, e, "Error get encoded CRL");
	}	
}

Handle<std::string> CRL::getSignature(){
	LOGGER_FN();

	try{
		std::string sslbuf = std::string((char *)(this->internal()->signature)->data, (this->internal()->signature)->length);
		Handle<std::string> res = new std::string(sslbuf.c_str(), sslbuf.length());

		return res;
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, CRL, e, "Error get signature CRL");
	}
}

Handle<std::string> CRL::getNextUpdate(){
	LOGGER_FN();

	LOGGER_OPENSSL(X509_CRL_get_nextUpdate);
	ASN1_TIME *time = X509_CRL_get_nextUpdate(this->internal());
	return ASN1_TIME_toString(time);
}

Handle<std::string> CRL::getThisUpdate(){
	LOGGER_FN();

	LOGGER_OPENSSL(X509_CRL_get_lastUpdate);
	ASN1_TIME *time = X509_CRL_get_lastUpdate(this->internal());
	return ASN1_TIME_toString(time);
}


Handle<RevokedCertificate> CRL::getRevokedCertificate(Handle<Certificate> cert){
	LOGGER_FN();

	if (cert.isEmpty()){
		THROW_EXCEPTION(0, CRL, NULL, ERROR_PARAMETER_NULL, 1);
	}		
	X509_REVOKED *rc = NULL;
	LOGGER_OPENSSL(X509_CRL_get0_by_cert);
	if (X509_CRL_get0_by_cert(this->internal(), &rc, cert->internal())){
		return new RevokedCertificate(rc, this->handle());
	}
	else{
		return NULL;
	}
}

Handle<RevokedCertificate> CRL::getRevokedCertificate(Handle<std::string> serial){
	LOGGER_FN();

	if (serial.isEmpty()){
		THROW_EXCEPTION(0, CRL, NULL, ERROR_PARAMETER_NULL, 1);
	}	
	
	BIGNUM* bn = BN_new();
	BN_bin2bn((const unsigned char *)serial->data(), serial->length(), bn);

	ASN1_INTEGER* asnInt = ASN1_INTEGER_new();
	BN_to_ASN1_INTEGER(bn, asnInt);

	X509_REVOKED *rc = NULL;	
	LOGGER_OPENSSL(X509_CRL_get0_by_serial);
	if (X509_CRL_get0_by_serial(this->internal(), &rc, asnInt)){
		return new RevokedCertificate(rc, this->handle());
	}
	else{
		return NULL;
	}
}

Handle<std::string> RevokedCertificate::revocationDate()
{
	LOGGER_FN();

	ASN1_TIME *time = this->internal()->revocationDate;
	return ASN1_TIME_toString(time);
}

int RevokedCertificate::reason()
{
	LOGGER_FN();

	return this->internal()->reason;
}
