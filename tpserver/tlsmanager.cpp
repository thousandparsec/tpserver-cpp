/*  TlsManager class
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <gnutls/gnutls.h>
#include <stdint.h>

#include "settings.h"
#include "logging.h"

#include "tlsmanager.h"

TlsManager* TlsManager::instance = NULL;

TlsManager* TlsManager::getInstance(){
    if(instance == NULL){
        instance = new TlsManager();
    }
    return instance;
}

void TlsManager::reference(){
    if(refcount == 0){
        Settings* settings = Settings::getSettings();
        gnutls_global_init ();
        gnutls_certificate_allocate_credentials (&credentials);
        if(settings->get("x509_tls") != ""){
            gnutls_certificate_set_x509_trust_file (credentials, settings->get("x509_trust_file").c_str(), GNUTLS_X509_FMT_PEM);

            //gnutls_certificate_set_x509_crl_file (credentials, settings->get("x509_crl_file").c_str(), GNUTLS_X509_FMT_PEM);

            gnutls_certificate_set_x509_key_file (credentials, settings->get("x509_cert_file").c_str(),
                    settings->get("x509_key_file").c_str(), GNUTLS_X509_FMT_PEM);

        }
        gnutls_dh_params_init (&dh_params);
        gnutls_dh_params_generate2 (dh_params, 1024);

        gnutls_certificate_set_dh_params (credentials, dh_params);
    }
    refcount++;
}

void TlsManager::dereference(){
    if(refcount > 0){
        refcount--;
        if(refcount == 0){
            gnutls_certificate_free_credentials (credentials);
            gnutls_dh_params_deinit (dh_params);
            gnutls_global_deinit ();
        }
    }
}

gnutls_certificate_credentials_t TlsManager::getCredentials(){
    return credentials;
}

TlsManager::TlsManager(): refcount(0){
}
