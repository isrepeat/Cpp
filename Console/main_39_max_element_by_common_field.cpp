//#include <map>
//#include <vector>
//#include <string>
//#include <algorithm>
//#include <iterator>
//
//
//struct Purchase {
//	std::string purchaseId;
//	int duration;
//};
//
//struct PurchaseDescription {
//	std::string purchaseId;
//	std::string name;
//};
//
//void main() {
//	std::vector<Purchase> allPurchases = {
//		{"id_111", 90},
//		{"id_222", 30},
//		{"id_333", 365},
//		{"id_444", 180},
//	};
//
//	std::vector<PurchaseDescription> boughtPurchases = {
//		{"id_444", "6 months"},
//		{"id_222", "1 month"},
//	};
//
//
//	struct PurchasePairRef {
//		PurchaseDescription& refPurchaseDescription;
//		Purchase& refPurchase;
//	};
//
//	std::vector<PurchasePairRef> purchasesData;
//
//	for (auto& boughtPurchase : boughtPurchases) {
//		auto purchaseIt = std::find_if(allPurchases.begin(), allPurchases.end(), [&boughtPurchase](const Purchase& purchase) {
//			return purchase.purchaseId == boughtPurchase.purchaseId;
//			});
//
//		purchasesData.push_back({
//			boughtPurchase,
//			*purchaseIt
//			});
//	}
//
//	auto maxPurchase = std::max_element(purchasesData.begin(), purchasesData.end(), [](const PurchasePairRef& A, const PurchasePairRef& B) {
//		return A.refPurchase.duration < B.refPurchase.duration;
//		});
//
//	auto result = maxPurchase->refPurchaseDescription;
//
//	int xx = 9;
//
//	return;
//}