import { BrowserRouter, Routes, Route } from 'react-router-dom';
import { Layout } from './components/layout/Layout';
import { HomePage } from './pages/HomePage';
import { AboutPage } from './pages/AboutPage';
import { PlatformPage } from './pages/PlatformPage';
import { ProductsPage } from './pages/ProductsPage';
import { ProductSFSVCPage } from './pages/ProductSFSVCPage';
import { ProductNERMNPage } from './pages/ProductNERMNPage';
import { ProductNSSIMPage } from './pages/ProductNSSIMPage';
import { ResourcesPage } from './pages/ResourcesPage';
import { CareersPage } from './pages/CareersPage';
import { ContactPage } from './pages/ContactPage';
import { FAQPage } from './pages/FAQPage';
export default function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route element={<Layout />}>
          <Route index element={<HomePage />} />
          <Route path="about" element={<AboutPage />} />
          <Route path="platform" element={<PlatformPage />} />
          <Route path="products" element={<ProductsPage />} />
          <Route path="products/sfsvc" element={<ProductSFSVCPage />} />
          <Route path="products/nermn" element={<ProductNERMNPage />} />
          <Route path="products/nssim" element={<ProductNSSIMPage />} />
          <Route path="resources" element={<ResourcesPage />} />
          <Route path="careers" element={<CareersPage />} />
          <Route path="contact" element={<ContactPage />} />
          <Route path="faq" element={<FAQPage />} />
        </Route>
      </Routes>
    </BrowserRouter>
  );
}
