#include "privacydialog.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>

PrivacyDialog::PrivacyDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("Set"));
    initUi();
}
PrivacyDialog::~PrivacyDialog()
{

}

void PrivacyDialog::initUi()
{
    this->resize(560 , 560);
    QVBoxLayout *mverticalLayout = new QVBoxLayout(this);
    mverticalLayout->setSpacing(8);
    mverticalLayout->setContentsMargins(24, 0, 24, 24);

    QLabel *mTitleLabel = new QLabel(QString(tr("End User License Agreement and Privacy Policy Statement of Kylin")) , this);
    mTitleLabel->setAlignment(Qt::AlignCenter);
    mverticalLayout->addWidget(mTitleLabel);
    QTextBrowser *mContentLabel = new QTextBrowser(this);
     mContentLabel->setText(tr("Dear users of Kylin operating system and relevant products,\n\n   "
                                "Please read the clauses of the Agreement and the supplementary license (hereinafter collectively referred to as “the Agreement”)"
                                " and the privacy policy statement for Kylin operating system (hereinafter referred to as “the Statement”). When you click the next "
                                 "step to confirm your complete understanding of the content, it shall indicate that you have accepted the clauses of the Agreement,"
                                 " and the Agreement shall take effect immediately and be legally binding on you and the Company.\n   "
                                "“This product” in the Agreement and the Statement refers to “Kylin operating system software product” developed, produced and "
                                "released by Kylinsoft Co., Ltd. and used for handling the office work or building the information infrastructure for enterprises and "
                                "governments. “We” refers to Kylinsoft Co., Ltd. “You” refers to the users who pay the license fee and use the Kylin operating system"
                                " and relevant products. \n\n"
                                "End User License Agreement of Kylin \nRelease date of the version: July 30, 2021\nEffective date of the version: July 30, 2021 \n\n"
                                "The Agreement shall include the following content:\nI. 	User license \nII. 	Java technology limitations\nIII. 	Cookies and other te"
                                "chnologies\nIV. 	Intellectual property clause\nV. 	Open source code\n"
                                "VI. 	The third-party software/services\nVII. Escape clause\nVIII. Integrity and severability of the Agreement\nIX. 	Applicable law "
                                "and dispute settlement\n\n   I. 	User license\n\n   "
                                "According to the number of users who have paid for this product and the types of computer hardware, we shall grant the non-exclusive"
                               " and non-transferable license to you, and shall only allow the licensed unit and the employees signing the labor contracts with the "
                               "unit to use the attached software (hereinafter referred to as “the Software”) and documents as well as any error correction provided "
                               "by Kylinsoft.\n   1.	User license for educational institutions\n   In the case of observing the clauses and conditions of the Agreement,"
                               " if you are an educational institution, your institution shall be allowed to use the attached unmodified binary format software and "
                               "only for internal use. “For internal use” here refers to that the licensed unit and the employees signing the labor contracts with the "
                               "unit as well as the students enrolled by your institution can use this product. \n   2.	Use of the font software\n   Font software refers"
                               " to the software pre-installed in the product and generating font styles. You cannot separate the font software from the Software "
                               "and cannot modify the font software in an attempt to add any function that such font software, as a part of this product, does not"
                               " have when it is delivered to you, or you cannot embed the font software in the files provided as a commercial product for any fee "
                               "or other remuneration, or cannot use it in equipment where this product is not installed. If you use the font software for other "
                               "commercial purposes such as external publicity, please contact and negotiate with the font copyright manufacture to obtain the"
                               " permissions for your relevant acts.\n\n   II. 	Java technology limitations \n\n   You cannot change the “Java Platform Interface”"
                               " (referred to as “JPI”, that is, the classes in the “java” package or any sub-package of the “java” package), whether by creating additional"
                               " classes in JPI or by other means to add or change the classes in JPI. If you create an additional class as well as one or multiple relevant "
                               "APIs, and they (i) expand the functions of Java platform; And (ii) may be used by the third-party software developers to develop additional "
                               "software that may call the above additional APIs, you must immediately publish the accurate description of such APIs widely for free use "
                               "by all developers. You cannot create or authorize other licensees to create additional classes, interfaces or sub-packages marked as “java”,"
                               " “javax” and “sun” in any way, or similar agreements specified by Sun in any naming agreements. See the appropriate version of the Java"
                               " Runtime Environment Binary Code License (located at http://jdk.java.net at present) to understand the availability of runtime code jointly"
                               " distributed with Java mini programs and applications.\n\n   III. 	Cookies and other technologies\n\n   "
                                "In order to help us better understand and serve the users, our website, online services and applications may use the “Cookie” technology. "
                               "Such Cookies are used to store the network traffic entering and exiting the system and the traffic generated due to detection errors, so "
                               "they must be set. We shall understand how you interact with our website and online services by using such Cookies.\n   If you want to"
                               " disable the Cookie and use the Firefox browser, you may set it in Privacy and Security Center of Firefox. If your use other browsers, please "
                               "consult the specific schemes from the relevant suppliers. \n   In accordance with Article 76, paragraph 5 of the Network Security Law of the"
                               " People's Republic of China, personal information refers to all kinds of information recorded in electronic or other ways, which can identify "
                               "the natural persons’ personal identity separately or combined with other information, including but not limited to the natural person’s name,"
                               " date of birth, identity certificate number, personal biological identification information, address and telephone number, etc. If Cookies "
                               "contain the above information, or the combined information of non-personal information and other personal information collected through "
                               "Cookie, for the purpose of this privacy policy, we shall regard the combined information as personal privacy information, and shall provide "
                               "the corresponding security protection measures for your personal information by referring to Kylin Privacy Policy Statement.\n\n   "
                               "IV. 	Intellectual property clause\n\n   1.	Trademarks and Logos\n   This product shall be protected by the copyright law, trademark law and "
                               "other laws and international intellectual property conventions. Title to the product and all associated intellectual property rights are"
                               " retained by us or its licensors. No right, title or interest in any trademark, service mark, logo or trade name of us or its licensors is granted "
                               "under the Agreement. Any use of Kylinsoft marked by you shall be in favor of Kylinsoft, and without our consent, you shall not arbitrarily "
                               "use any trademark or sign of Kylinsoft.\n  2.	Duplication, modification and distribution\n   If the Agreement remains valid for all duplicates,"
                               " you may and must duplicate, modify and distribute software observing GNU GPL-GNU General Public License agreement among the Kylin "
                               "operating system software products in accordance with GNU GPL-GNU General Public License, and must duplicate, modify and distribute "
                               "other Kylin operating system software products not observing GNU GPL-GNU General Public License agreement in accordance with relevant"
                               " laws and other license agreements, but no derivative release version based on the Kylin operating system software products can use any"
                               " of our trademarks or any other signs without our written consent.\n   Special notes: Such duplication, modification and distribution shall "
                               "not include any software, to which GNU GPL-GNU General Public License does not apply, in this product, such as the software store, input"
                               " method software, font library software and third-party applications contained by the Kylin operating system software products. You shall "
                               "not duplicate, modify (including decompilation or reverse engineering) or distribute the above software unless prohibited by applicable "
                               "laws. \n\n   V. 	Open source code\n\n   For any open source codes contained in this product, any clause of the Agreement shall not limit, "
                               "constrain or otherwise influence any of your corresponding rights or obligations under any applicable open source code license or all kinds "
                               "of conditions you shall observe.\n\n   VI.  The third-party software/services\n\n   The third-party software/services referred to in the Agreement"
                               " refer to relevant software/services developed by other organizations or individuals other than the Kylin operating system manufacturer. This "
                               "product may contain or be bundled with the third-party software/services to which the separate license agreements are attached. When you "
                               "use any third-party software/services with separate license agreements, you shall be bound by such separate license agreements.\n   We shall"
                               " not have any right to control the third-party software/services in these products and shall not expressly or implicitly ensure or guarantee the"
                               " legality, accuracy, effectiveness or security of the acts of their providers or users.\n\n   VII. 	Escape clause\n\n   1.	Limited warranty\n    We "
                               "guarantee to you that within ninety (90) days from the date when you purchase or obtain this product in other legal ways (subject to the date "
                               "of the sales contract), the storage medium (if any) of this product shall not be involved in any defects in materials or technology when it is "
                               "normally used. All compensation available to you and our entire liability under this limited warranty will be for us to choose to replace this "
                               "product media or refund the fee paid for this product.\n   2.	Disclaimer\n   In addition to the above limited warranty, the Software is provided"
                               " “as is” without any express or implied condition statement and warranty, including any implied warranty of merchantability, suitability for a "
                               "particular purpose or non-infringement, except that this disclaimer is deemed to be legally invalid.\n   3.	Limitation of responsibility\n   To the"
                               " extent permitted by law, under any circumstances, no matter what theory of liability is adopted, no matter how it is caused, for any loss of "
                               "income, profit or data caused by or related to the use or inability to use the Software, or for special indirect consequential incidental or punitive"
                               " damages, neither we nor its licensors shall be liable (even if we have been informed of the possibility of such damages). According to the"
                               " Agreement, in any case, whether in contract tort (including negligence) or otherwise, our liability to you will not exceed the amount you pay "
                               "for the Software. The above limitations will apply even if the above warranty fails of its essential purpose.\n\n   VIII. 	Integrity and severability "
                               "of the Agreement\n\n   1.	The integrity of the Agreement\n  The Agreement is an entire agreement on the product use concluded by us with "
                               "you. It shall replace all oral or written contact information, suggestions, representations and guarantees inconsistent with the Agreement "
                               "previous or in the same period. During the period of the Agreement, in case of any conflict clauses or additional clauses in the relevant "
                               "quotations, orders or receipts or in other correspondences regarding the content of the Agreement between the parties, the Agreement shall"
                               " prevail. No modification of the Agreement will be binding, unless in writing and signed by an authorized representative of each party.\n   2."
                               "	Severability of the Agreement\n   If any provision of the Agreement is deemed to be unenforceable, the deletion of the corresponding "
                               "provision will still be effective, unless the deletion will hinder the realization of the fundamental purpose of the parties (in which case, the"
                               " Agreement will be terminated immediately).\n\n   IX. 	Applicable law and dispute settlement\n\n   1.	Application of governing laws\n   Any"
                               " dispute settlement (including but not limited to litigation and arbitration) related to the Agreement shall be governed by the laws of the "
                               "People’s Republic of China. The legal rules of any other countries and regions shall not apply.\n   2.	Termination\n   If the Software becomes"
                               " or, in the opinion of either party, may become the subject of any claim for intellectual property infringement, either party may terminate "
                               "the Agreement immediately.\n   The Agreement is effective until termination. You may terminate the Agreement at any time, but you must "
                               "destroy all originals and duplicates of the Software. The Agreement will terminate immediately without notice from us if you fail to comply "
                               "with any provision of the Agreement. At the time of termination, you must destroy all originals and duplicates of such software, and shall"
                               " be legally liable for not observing the Agreement.\n\n   The Agreement shall be in both Chinese and English, and in case of ambiguity "
                               "between any content above, the Chinese version shall prevail.\n\nPrivacy Policy Statement of Kylin Operating System\nRelease date of "
                               "the version: July 30, 2021\nEffective date of the version: July 30, 2021\n\nWe attach great importance to personal information and privacy "
                               "protection. In order to guarantee the legal, reasonable and appropriate collection, storage and use of your personal privacy information"
                               " and the transmission and storage in the safe and controllable circumstances, we hereby formulate this Statement. We shall provide your "
                               "personal information with corresponding security protection measures according to the legal requirements and mature security standards"
                               " in the industry.\n   The Statement shall include the following content:\n    I. 	Collection and use your personal information\n   II. 	How to "
                               "store and protect your personal information\n   III. 	How to manage your personal information\n   IV. 	Privacy of the third-party software/"
                               "services\n   V. 	Minors’ use of the products\n   VI. 	How to update this Statement\n   VII. How to contact us\n\n    I. 	How to collect and use your"
                               " personal information\n\n   1.	The collection of personal information\n   We shall collect the relevant information when you use this "
                               "product mainly to provide you with higher-quality products, more usability and better services. Part of information collected shall be "
                               "provided by you directly, and other information shall be collected by us through your interaction with the product as well as your use "
                               "and experience of the product. We shall not actively collect and deal with your personal information unless we have obtained your express "
                               "consent according to the applicable legal stipulations.\n   1)	The licensing mechanism for this product allows you to apply for the formal"
                               " license of the product in accordance with the contract and relevant agreements after you send a machine code to the commercial personnel "
                               "of Kylinsoft, and the machine code is generated through encryption and conversion according to the information of the computer used "
                               "by you, such as network card, firmware and motherboard. This machine code shall not directly contain the specific information of the "
                               "equipment, such as network card, firmware and motherboard, of the computer used by you.\n   2)	Server of the software store of this "
                               "product shall connect it according to the CPU type information and IP address of the computer used by you; at the same time, we shall "
                               "collect the relevant information of your use of the software store of this product, including but not limited to the time of opening the software "
                               "store, interaction between the pages, search content and downloaded content. The relevant information collected is generally recorded in "
                               "the log of server system of software store, and the specific storage position may change due to different service scenarios.\n   3)	Upgrading "
                               "and updating of this product shall be connected according to the IP address of the computer used by you, so that you can upgrade and "
                               "update the system;\n   4)	Your personal information, such as E-mail address, telephone number and name, shall be collected due to business"
                               " contacts and technical services.\n   5)	The biological characteristic management tool support system components of this product shall use"
                               " the biological characteristics for authentication, including fingerprint, finger vein, iris and voiceprint. The biological characteristic information"
                               " input by you shall be stored in the local computer, and for such part of information, we shall only receive the verification results but shall not "
                               "collect or upload it. If you do not need to use the biological characteristics for the system authentication, you may disable this function in the"
                               " biological characteristic management tool.\n   6)	This product shall provide the recording function. When you use the recording function of "
                               "this product, we shall only store the audio content when you use the recording in the local computer but shall not collect or upload the content.\n"
                               "   7)	The service and support functions of this product shall collect the information provided by you for us, such as log, E-mail, telephone and "
                               "name, so as to make it convenient to provide the technical services, and we shall properly keep your personal information.\n   8)	In the upgrading"
                               " process of this product, if we need to collect additional personal information of yours, we shall timely update this part of content.\n\n  2.	Use of "
                               "personal information\n   We shall strictly observe the stipulations of laws and regulations and agreements with you to use the information collected"
                               " for the following purposes. In case of exceeding the scope of following purposes, we shall explain to you again and obtain your consent.\n   1)	"
                               "The needs such as product licensing mechanism, use of software store, system updating and maintenance, biological identification and online"
                               " services shall be involved;\n   2)	We shall utilize the relevant information to assist in promoting the product security, reliability and sustainable"
                               " service;\n   3)	We shall directly utilize the information collected (such as the E-mail address and telephone provided by you) to communicate "
                               "with you directly, for example, business contact, technical support or follow-up service visit;\n   4)	We shall utilize the data collected to improve "
                               "the current usability of the product, promote the product’s user experience (such as the personalized recommendation of software store) and"
                               " repair the product defects, etc.;\n   5)	We shall use the user behavior data collected for data analysis. For example, we shall use the information"
                               " collected to analyze and form the urban thermodynamic chart or industrial insight report excluding any personal information. We may make the"
                               " information excluding identity identification content upon the statistics and processing public and share it with our partners, to understand how"
                               " the users use our services or make the public understand the overall use trend of our services;\n   6)	We may use your relevant information and"
                               " provide you with the advertising more related to you on relevant websites and in applications and other channels;\n   7)	In order to follow "
                               "the relevant requirements of relevant laws and regulations, departmental regulations and rules and governmental instructions.\n\n   3.	Information"
                               " sharing and provision\n   We shall not share or transfer your personal information to any third party, except for the following circumstances:\n   1)"
                               "	After obtaining your clear consent, we shall share your personal information with the third parities;\n   2)	In order to achieve the purpose of"
                               " external processing, we may share your personal information with the related companies or other third-party partners (the third-party service"
                               " providers, contractors, agents and application developers). We shall protect your information security by means like encryption and anonymization;"
                               "\n    3)	We shall not publicly disclose the personal information collected. If we must disclose it publicly, we shall notify you of the purpose of such "
                               "public disclosure, type of information disclosed and the sensitive information that may be involved, and obtain your consent;\n   4)	With the continuous"
                               " development of our business, we may carry out the transactions, such as merger, acquisition and asset transfer, and we shall notify you of the "
                               "relevant circumstances, and continue to protect or require the new controller to continue to protect your personal information according to "
                               "laws and regulations and the standards no lower than that required by this Statement;\n    5)	If we use your personal information beyond the "
                               "purpose claimed at the time of collection and the directly or reasonably associated scope, we shall notify you again and obtain your consent before"
                               " using your personal information.\n\n   4.	Exceptions with authorized consent\n   1)	It is directly related to national security, national defense security"
                               " and other national interests;\n   2)	It is directly related to public safety, public health and public knowledge and other major public interests;"
                               "\n   3)	It is directly related to crime investigation, prosecution, judgment and execution of judgment;\n   4)	It aims to safeguard the life, property and "
                               "other major legal rights and interests of you or others but it is impossible to obtain your own consent;\n   5)	The personal information collected is "
                               "disclosed to the public by yourself;\n   6)	Personal information collected from legally publicly disclosed information, such as legal news reports,"
                               " government information disclosure and other channels;\n   7)	It is necessary to sign and perform of the contract according to your requirement;"
                               "\n   8)	It is necessary to maintain the safe and stable operation of the provided products or services, including finding and handling any fault of "
                               "products or services;\n   9)	It is necessary to carry out statistical or academic research for public interest, and when the results of academic research"
                               " or description are provided, the personal information contained in the results is de-identified;\n   10)	Other circumstances specified in the laws"
                               " and regulations.\n\n   II. 	How to store and protect personal information\n\n   1.	Information storage place\n   We shall store the personal"
                               " information collected and generated in China within the territory of China in accordance with laws and regulations.\n   2.	Information storage"
                               " duration\n  Generally speaking, we shall retain your personal information for the time necessary to achieve the purpose or for the shortest term "
                               "stipulated by laws and regulations. Information recorded in the log shall be kept for a specified period and be automatically deleted according"
                               " to the configuration.\n   When operation of our product or services stops, we shall notify you in the forms such as notification and announcement,"
                               " delete your personal information or conduct anonymization within a reasonable period and immediately stop the activities collecting the personal"
                               " information.\n   3.	How to protect the information\n   We shall strive to provide guarantee for the users’ information security, to prevent the loss, "
                               "improper use, unauthorized access or disclosure of the information.\n   We shall use the security protection measures within the reasonable security"
                               " level to protect the information security. For example, we shall protect your system account and password by means like encryption.\n   We shall"
                               " establish the special management systems, processes and organizations to protect the information security. For example, we shall strictly restrict"
                               " the scope of personnel who access to the information, and require them to observe the confidentiality obligation.\n   4.	Emergency response plan\n"
                               "    In case of security incidents, such as personal information disclosure, we shall start the emergency response plan according to law, to prevent "
                               "the security incidents from spreading, and shall notify you of the situation of the security incidents, the possible influence of the incidents on you"
                               " and the remedial measures we will take, in the form of pushing the notifications and announcements. We will also report the disposition of the"
                               " personal information security events according to the laws, regulations and regulatory requirements.\n\n   III. 	How to manage your personal "
                               "information\n\n   If you worry about the personal information disclosure caused by using this product, you may consider suspending or not using"
                               " the relevant functions involving the personal information, such as the formal license of the product, application store, system updating and "
                               "upgrading and biological identification, according to the personal and business needs.\n   Please pay attention to the personal privacy protection "
                               "at the time of using the third-party software/services in this product.\n\n   IV. 	Privacy of the third-party software/services\n\n   The third-party "
                               "software/services referred to in the Agreement refer to relevant software/services developed by other organizations or individuals other than "
                               "the Kylin operating system manufacturer.\n   When you install or use the third-party software/services in this product, the privacy protection "
                               "and legal responsibility of the third-party software/services shall be independently borne by the third-party software/services. Please carefully"
                               " read and examine the privacy statement or clauses corresponding to the third-party software/services, and pay attention to the personal privacy "
                               "protection.\n\n   V. 	Minors’ use of the products\n\n   If you are a minor, you shall obtain your guardian’s consent on your use of this product and "
                               "the relevant service clauses. Except for the information required by the product, we shall not deliberately require the minors to provide more data. "
                               "With the guardians’ consent or authorization, the accounts created by the minors shall be deemed to be the same as any other accounts. We have "
                               "formulated special information processing rules to protect the personal information security of minors using this product. The guardians shall "
                               "also take the appropriate preventive measures to protect the minors and supervise their use of this product.\n\n   VI. 	How to update this Statement\n\n"
                               "   We may update this Statement at any time, and shall display the updated statement to you through the product installation process or the company’s"
                               " website at the time of updating. After such updates take effect, if you use such services or any software permitted according to such clauses, you "
                               "shall be deemed to agree on the new clauses. If you disagree on the new clauses, then you must stop using this product, and please close the "
                               "account created by you in this product; if you are a parent or guardian, please help your minor child to close the account created by him/her"
                               " in this product.\n\n   VII. 	How to contact us\n\n  If you have any question, or any complaints or opinions on this Statement, you may seek "
                               "advice through our customer service hotline 400-089-1870, or the official website (www.kylinos.cn), or “service and support” application "
                               "in this product. You may also contact us by E-mail (market@kylinos.cn).\n   We shall timely and properly deal with them. Generally, a reply "
                               "will be made within 15 working days.\n   The Statement shall take effect from the date of updating. The Statement shall be in Chinese and"
                               " English at the same time and in case of any ambiguity of any clause above, the Chinese version shall prevail.\n   Last date of updating: November "
                               "1, 2021\n\nAddress: Building 3, Xin’an Entrepreneurship Plaza, Tanggu Marine Science and Technology Park, Binhai High-tech Zone, Tianjin (300450)\n"
                               "             Silver Valley Tower, No. 9, North Forth Ring West Road, Haidian District, Beijing (100190)\n             Building T3, Fuxing World Financial Center, No. "
                               "303, Section 1 of Furong Middle Road, Kaifu District, Changsha City (410000)\n             Digital Entertainment Building, No. 1028, Panyu Road, Xuhui District,"
                               " Shanghai (200030)\nTel.: Tianjin (022) 58955650       Beijing (010) 51659955\n             Changsha (0731) 88280170    Shanghai (021) 51098866\nFax: Tianjin"
                               " (022) 58955651       Beijing (010) 62800607\n             Changsha (0731) 88280166    Shanghai (021) 51062866\n"
                            "\nCompany website: www.kylinos.cn\nE-mail: support@kylinos.cn"));
    mverticalLayout->addWidget(mContentLabel);
    QLabel *label_1 = new QLabel(QString(tr("Kylinsoft Co., Ltd.")) , this);
    label_1->setAlignment(Qt::AlignRight);
    mverticalLayout->addWidget(label_1);
}








