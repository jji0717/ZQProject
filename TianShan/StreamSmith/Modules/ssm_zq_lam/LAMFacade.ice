module com {
	module izq {
		module am {
			module facade {
				module servicesForIce {

					struct AEInfo {
						string aeUID;			// the uid of asset element
						int    bandWidth;		// bandWidth
						int    cueIn;
						int    cueOut;
					};

					// the collection of AE is a sequence or vector
					sequence<AEInfo> AssetElementCollection;

			        interface LAMFacade {
			        	AssetElementCollection getAEList(int assetUID);
			            AssetElementCollection getAEListWithAppUID(int assetUID, string appUID);
			            AssetElementCollection getAEListByProviderIdAndPAssetId(string providerId, string providerAUID);
			        };					
				};
	    	};
		};
	};

}; 
