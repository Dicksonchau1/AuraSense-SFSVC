import { BrowserRouter, Routes, Route } from 'react-router-dom';
import { Layout } from './components/layout/Layout';
import { HomePage } from './pages/HomePage';
import { PlatformPage } from './pages/PlatformPage';
import { ProductsPage } from './pages/ProductsPage';
import { SFSVCPage } from './pages/SFSVCPage';
import { NERMNPage } from './pages/NERMNPage';
import { NSSIMPage } from './pages/NSSIMPage';
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
          <Route path="platform" element={<PlatformPage />} />
          <Route path="products" element={<ProductsPage />} />
          <Route path="products/sfsvc" element={<SFSVCPage />} />
          <Route path="products/nermn" element={<NERMNPage />} />
          <Route path="products/nssim" element={<NSSIMPage />} />
          <Route path="resources" element={<ResourcesPage />} />
          <Route path="careers" element={<CareersPage />} />
          <Route path="contact" element={<ContactPage />} />
          <Route path="faq" element={<FAQPage />} />
        </Route>
      </Routes>
    </BrowserRouter>
  );
}
