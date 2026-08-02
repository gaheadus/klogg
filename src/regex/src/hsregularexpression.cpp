*** Begin Patch
*** Update File: src/regex/src/hsregularexpression.cpp
@@
-            klogg::vector<unsigned> flags( expressions.size() );
-            std::transform( expressions.cbegin(), expressions.cend(), flags.begin(),
+            klogg::vector<unsigned> flags( expressions.size() );
+            if ( expressions.empty() ) {
+                // Protect against calling hs_compile_multi with empty expressions
+                // which triggers hyperscan error "expressions is NULL".
+                LOG_ERROR << "Failed to compile pattern: expressions is NULL";
+                errorMessage = QStringLiteral("expressions is NULL");
+                return nullptr;
+            }
+
+            std::transform( expressions.cbegin(), expressions.cend(), flags.begin(),
                             [ isPrefilter ]( const auto& expression ) {
@@
-            klogg::vector<const char*> patternPointers( utf8Patterns.size() );
-            std::transform( utf8Patterns.cbegin(), utf8Patterns.cend(), patternPointers.begin(),
-                            []( const auto& utf8Pattern ) { return utf8Pattern.data(); } );
+            klogg::vector<const char*> patternPointers( utf8Patterns.size() );
+            std::transform( utf8Patterns.cbegin(), utf8Patterns.cend(), patternPointers.begin(),
+                            []( const auto& utf8Pattern ) { return utf8Pattern.data(); } );
@@
-    auto matcherScratch = makeUniqueResource<hs_scratch_t, hs_free_scratch>(
-        []( hs_scratch_t* prototype ) -> hs_scratch_t* {
-            hs_scratch_t* scratch = nullptr;
-
-            const auto err = hs_clone_scratch( prototype, &scratch );
-            if ( err != HS_SUCCESS ) {
-                LOG_ERROR << "hs_clone_scratch failed";
-                return nullptr;
-            }
-
-            return scratch;
-        },
-        scratch_.get() );
+    auto matcherScratch = makeUniqueResource<hs_scratch_t, hs_free_scratch>(
+        []( hs_scratch_t* prototype ) -> hs_scratch_t* {
+            hs_scratch_t* scratch = nullptr;
+
+            const auto err = hs_clone_scratch( prototype, &scratch );
+            if ( err != HS_SUCCESS ) {
+                LOG_ERROR << "hs_clone_scratch failed";
+                return nullptr;
+            }
+
+            return scratch;
+        },
+        scratch_.get() );
+
+    if ( !matcherScratch ) {
+        LOG_ERROR << "Failed to create matcher scratch — falling back to noop matcher";
+        return HsNoopMatcher();
+    }
@@
 MatchedPatterns HsSingleMatcher::match( const std::string_view& utf8Data ) const
 {
     context_.reset();
-
-    hs_scan( database_.get(), utf8Data.data(), static_cast<unsigned int>( utf8Data.size() ), 0,
-             scratch_.get(), matchSingleCallback, static_cast<void*>( &context_ ) );
-
-    return std::move( context_.matchingPatterns );
+
+    // Defensive: ensure database and scratch are valid before calling hs_scan
+    if ( !database_.get() || !scratch_.get() ) {
+        LOG_ERROR << "HsSingleMatcher::match called with invalid database or scratch";
+        return std::move( context_.matchingPatterns );
+    }
+
+    hs_scan( database_.get(), utf8Data.data(), static_cast<unsigned int>( utf8Data.size() ), 0,
+             scratch_.get(), matchSingleCallback, static_cast<void*>( &context_ ) );
+
+    return std::move( context_.matchingPatterns );
 }
@@
 MatchedPatterns HsMultiMatcher::match( const std::string_view& utf8Data ) const
 {
     context_.reset();
-
-    hs_scan( database_.get(), utf8Data.data(), static_cast<unsigned int>( utf8Data.size() ), 0,
-             scratch_.get(), matchMultiCallback, static_cast<void*>( &context_ ) );
-
-    return std::move( context_.matchingPatterns );
+
+    if ( !database_.get() || !scratch_.get() ) {
+        LOG_ERROR << "HsMultiMatcher::match called with invalid database or scratch";
+        return std::move( context_.matchingPatterns );
+    }
+
+    hs_scan( database_.get(), utf8Data.data(), static_cast<unsigned int>( utf8Data.size() ), 0,
+             scratch_.get(), matchMultiCallback, static_cast<void*>( &context_ ) );
+
+    return std::move( context_.matchingPatterns );
 }
*** End Patch
